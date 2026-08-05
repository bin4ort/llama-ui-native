/**
 * api.js — HTTP client for the local llama.cpp backend + the native server.
 * Kernel addition (Phase 2, contract change): planned in REFACTOR-PLAN §2
 * but not built in Phase 1. Both verticals use it: Agent A (chat streaming,
 * props/slots) and Agent B (models list, wizard generation, health).
 *
 * Error codes are taken from the frozen registry (error-codes.js):
 * API-000/001/002 (generic), API-003 (models), API-004 (props), API-005
 * (slots), STR-000 (connect), STR-001 (SSE parse), STR-002 (aborted),
 * STR-004 (timeout), STR-005 (server error event).
 */
import { configStore } from './settings-store.js';
import { log, LlmUiError } from './logger.js';

export function baseUrl() {
  let endpoint = configStore.get().serverEndpoint || 'http://localhost:8080';
  endpoint = endpoint.trim().replace(/\/+$/, '');
  if (/\/\/(localhost|127\.0\.0\.1)/.test(endpoint) && !['localhost', '127.0.0.1'].includes(window.location.hostname)) {
    endpoint = endpoint.replace(/\/\/(localhost|127\.0\.0\.1)/, `//${window.location.hostname}`);
  }
  return endpoint;
}

async function request(path, options = {}, codes, timeoutMs = 15000) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), options.timeoutMs ?? timeoutMs);
  try {
    const res = await fetch(`${baseUrl()}${path}`, {
      ...options,
      signal: controller.signal,
      headers: { 'Content-Type': 'application/json', ...(options.headers || {}) }
    });
    if (!res.ok) throw new LlmUiError(codes.http, `${path} failed: HTTP ${res.status}`, res.status);
    return res;
  } catch (err) {
    if (err instanceof LlmUiError) throw err;
    if (err?.name === 'AbortError')
      throw new LlmUiError(codes.timeout, `${path} timed out`, `${timeoutMs}ms`);
    throw new LlmUiError(codes.connect, `${path} unreachable`, err?.message ?? String(err));
  } finally {
    clearTimeout(timer);
  }
}

const GEN = { connect: 'LLMUI-API-000', http: 'LLMUI-API-001', timeout: 'LLMUI-API-001' };

/** GET /v1/models — list of available models (LLMUI-API-003 on failure). */
export async function getModels() {
  const res = await request('/v1/models', {}, { ...GEN, connect: 'LLMUI-API-003', http: 'LLMUI-API-003', timeout: 'LLMUI-API-003' });
  const data = await res.json().catch(() => {
    throw new LlmUiError('LLMUI-API-002', 'models: response not JSON');
  });
  return Array.isArray(data?.data) ? data.data : [];
}

/** GET /props — server sampling props (LLMUI-API-004 on failure). */
export async function getProps() {
  const res = await request('/props', {}, { ...GEN, connect: 'LLMUI-API-004', http: 'LLMUI-API-004', timeout: 'LLMUI-API-004' });
  return res.json().catch(() => {
    throw new LlmUiError('LLMUI-API-002', 'props: response not JSON');
  });
}

/** GET /slots — active slots / model load state (LLMUI-API-005 on failure). */
export async function getSlots() {
  const res = await request('/slots', {}, { ...GEN, connect: 'LLMUI-API-005', http: 'LLMUI-API-005', timeout: 'LLMUI-API-005' });
  return res.json().catch(() => []);
}

/** GET /health on the native server (port 8765). */
export async function nativeHealth() {
  try {
    const res = await fetch('/health', { signal: AbortSignal.timeout(3000) });
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    return await res.json();
  } catch (err) {
    throw new LlmUiError('LLMUI-API-000', 'native health check failed', err?.message ?? String(err));
  }
}

/**
 * POST /v1/chat/completions — one-shot completion (no streaming).
 * Used by the preset wizard and title generation.
 */
export async function chatCompletion(messages, options = {}) {
  const body = {
    model: options.model ?? '',
    messages,
    temperature: options.temperature ?? 0.4,
    max_tokens: options.max_tokens ?? 1200,
    stream: false
  };
  const res = await request('/v1/chat/completions', { method: 'POST', body: JSON.stringify(body) }, GEN, 120000);
  const data = await res.json().catch(() => {
    throw new LlmUiError('LLMUI-API-002', 'completion: response not JSON');
  });
  const content = data?.choices?.[0]?.message?.content;
  if (typeof content !== 'string') {
    throw new LlmUiError('LLMUI-STR-006', 'completion: no content in response', data?.error?.message);
  }
  return content;
}

/**
 * POST /v1/chat/completions — streaming SSE reader.
 * Returns an async iterable of { delta, finish } chunks.
 */
export async function* streamChatCompletion(messages, options = {}) {
  const body = {
    model: options.model ?? '',
    messages,
    temperature: options.temperature ?? 0.7,
    max_tokens: options.max_tokens ?? -1,
    stream: true,
    ...(options.extra || {})
  };
  const res = await request(
    '/v1/chat/completions',
    { method: 'POST', body: JSON.stringify(body) },
    { connect: 'LLMUI-STR-000', http: 'LLMUI-STR-000', timeout: 'LLMUI-STR-004' },
    120000
  );
  const reader = res.body.getReader();
  const decoder = new TextDecoder();
  let buffer = '';
  try {
    while (true) {
      const { done, value } = await reader.read();
      if (done) break;
      buffer += decoder.decode(value, { stream: true });
      const lines = buffer.split('\n');
      buffer = lines.pop() ?? '';
      for (const line of lines) {
        const trimmed = line.trim();
        if (!trimmed.startsWith('data:')) continue;
        const payload = trimmed.slice(5).trim();
        if (payload === '[DONE]') {
          yield { delta: '', finish: 'stop' };
          return;
        }
        try {
          const parsed = JSON.parse(payload);
          const choice = parsed?.choices?.[0] ?? {};
          const d = choice.delta ?? {};
          const delta = d.content ?? '';
          const finish = choice.finish_reason;
          const extra = {};
          if (d.reasoning_content) extra.reasoning = d.reasoning_content;
          if (d.tool_calls?.length) extra.toolCalls = d.tool_calls;
          if (finish) yield { delta, finish, ...extra };
          else if (delta || extra.reasoning || extra.toolCalls) yield { delta, finish: null, ...extra };
        } catch {
          log.warn('LLMUI-STR-001', 'stream: unparseable SSE line', payload.slice(0, 120));
        }
      }
    }
  } finally {
    reader.releaseLock();
  }
  log.warn('LLMUI-STR-002', 'stream: closed without [DONE]');
}
