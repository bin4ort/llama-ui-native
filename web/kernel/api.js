/**
 * api.js — HTTP client: completions (SSE streaming), models, props, slots,
 * tools. Talks to the native C server proxy (same endpoints as today).
 * Error codes: LLMUI-API-*, LLMUI-STR-*.
 */
import { log, LlmUiError } from './logger.js';
import { config } from './settings-store.js';

const BASE = '/v1';

function authHeaders() {
  const key = String(config().apiKey ?? '').trim();
  return key ? { Authorization: `Bearer ${key}` } : {};
}

async function request(path, options = {}) {
  let response;
  try {
    response = await fetch(`${BASE}${path}`, {
      ...options,
      headers: { 'Content-Type': 'application/json', ...authHeaders(), ...(options.headers ?? {}) },
      signal: options.signal ?? AbortSignal.timeout(15000)
    });
  } catch (err) {
    throw new LlmUiError('LLMUI-API-000', 'http: request failed (network)', err?.message ?? String(err));
  }
  if (!response.ok) {
    if (response.status === 401 || response.status === 403) {
      throw new LlmUiError('LLMUI-API-009', 'api: unauthorized (401/403)', String(response.status));
    }
    throw new LlmUiError('LLMUI-API-001', 'http: non-2xx response', `${response.status}`);
  }
  return response;
}

export async function getModels() {
  const res = await request('/models');
  try {
    return await res.json();
  } catch (err) {
    throw new LlmUiError('LLMUI-API-003', 'api: models list fetch failed', err?.message);
  }
}

export async function getProps() {
  const res = await request('/props');
  try {
    return await res.json();
  } catch (err) {
    throw new LlmUiError('LLMUI-API-004', 'api: props fetch failed', err?.message);
  }
}

export async function getSlots() {
  const res = await request('/slots');
  try {
    return await res.json();
  } catch (err) {
    throw new LlmUiError('LLMUI-API-005', 'api: slots fetch failed', err?.message);
  }
}

export async function getTools() {
  const res = await request('/tools');
  try {
    return await res.json();
  } catch (err) {
    throw new LlmUiError('LLMUI-API-006', 'api: tools list fetch failed', err?.message);
  }
}

export async function executeTool(name, args) {
  const res = await request(`/tools/${encodeURIComponent(name)}`, {
    method: 'POST',
    body: typeof args === 'string' ? args : JSON.stringify(args)
  });
  try {
    return await res.json();
  } catch (err) {
    throw new LlmUiError('LLMUI-API-007', 'api: tool execute failed', err?.message);
  }
}

/**
 * Stream a chat completion. Emits callbacks as SSE frames arrive.
 * Events: { data } | { error } | { done }. The response is the OpenAI-shaped
 * delta object for each chunk.
 */
export async function streamChatCompletion({ messages, options = {}, onData, onDone, onError, signal }) {
  const body = {
    messages,
    stream: true,
    ...options
  };

  let response;
  try {
    response = await fetch(`${BASE}/chat/completions`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', ...authHeaders() },
      body: JSON.stringify(body),
      signal: signal ?? AbortSignal.timeout(120000)
    });
  } catch (err) {
    const e = new LlmUiError('LLMUI-STR-000', 'stream: connection failed', err?.message ?? String(err));
    onError?.(e);
    throw e;
  }

  if (!response.ok || !response.body) {
    let text = '';
    try {
      text = await response.text();
    } catch {
      /* ignore */
    }
    const e = new LlmUiError('LLMUI-STR-005', 'stream: server error event', `${response.status} ${text.slice(0, 200)}`);
    onError?.(e);
    throw e;
  }

  const reader = response.body.getReader();
  const decoder = new TextDecoder();
  let buffer = '';

  try {
    for (;;) {
      const { done, value } = await reader.read();
      if (done) break;
      buffer += decoder.decode(value, { stream: true });
      const frames = buffer.split('\n\n');
      buffer = frames.pop() ?? '';

      for (const frame of frames) {
        for (const line of frame.split('\n')) {
          if (!line.startsWith('data:')) continue;
          const payload = line.slice(5).trim();
          if (payload === '[DONE]') {
            onDone?.();
            return;
          }
          let json;
          try {
            json = JSON.parse(payload);
          } catch (err) {
            log.warn('LLMUI-STR-001', 'stream: SSE frame parse failed', payload.slice(0, 120));
            continue;
          }
          onData?.(json);
        }
      }
    }
  } catch (err) {
    if (err?.name === 'AbortError' || signal?.aborted) {
      const e = new LlmUiError('LLMUI-STR-002', 'stream: aborted mid-chunk');
      onError?.(e);
      throw e;
    }
    const e = new LlmUiError('LLMUI-STR-003', 'stream: resume failed', err?.message ?? String(err));
    onError?.(e);
    throw e;
  }

  onDone?.();
}
