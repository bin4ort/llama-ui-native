/**
 * tools.js — Agent A: built-in tool definitions + executors (ported from the
 * current app's builtin-tools.service.ts) and the agentic loop that runs
 * them. Permission prompts go through kernel.permissions.verify() (Agent B
 * registers the dialog). Error codes: LLMUI-TL-*, LLMUI-API-*.
 */
import * as kernel from '../../kernel/index.js';
import { streamChatCompletion } from '../../kernel/api.js';

const { config, log, permissions, t } = kernel;

const NAMES = {
  CALCULATE: 'calculate', FETCH_URL: 'fetch_url', TO_TABLE: 'to_table',
  JSON_TOOL: 'json_tool', CLIPBOARD: 'clipboard', NOTIFY: 'notify',
  TODO_LIST: 'todo_list', WEATHER: 'weather', WIKIPEDIA: 'wikipedia', PLOT_CHART: 'plot_chart'
};

const CONFIG_KEYS = {
  [NAMES.CALCULATE]: 'toolCalculateEnabled', [NAMES.FETCH_URL]: 'toolFetchUrlEnabled',
  [NAMES.TO_TABLE]: 'toolToTableEnabled', [NAMES.JSON_TOOL]: 'toolJsonEnabled',
  [NAMES.CLIPBOARD]: 'toolClipboardEnabled', [NAMES.NOTIFY]: 'toolNotifyEnabled',
  [NAMES.TODO_LIST]: 'toolTodoEnabled', [NAMES.WEATHER]: 'toolWeatherEnabled',
  [NAMES.WIKIPEDIA]: 'toolWikipediaEnabled', [NAMES.PLOT_CHART]: 'toolPlotChartEnabled'
};

const TOOL_SCHEMAS = {
  [NAMES.CALCULATE]: {
    description: 'Evaluate a mathematical expression exactly. Supports + - * / ^ % parentheses, factorial (!), functions sqrt, abs, sin, cos, tan, asin, acos, atan, ln, log10, exp, pow, round, floor, ceil, min, max. Constants pi, e.',
    params: { type: 'object', properties: { expression: { type: 'string' } }, required: ['expression'] }
  },
  [NAMES.FETCH_URL]: {
    description: 'Fetch a URL and return its visible text (HTML stripped). Capped at 1 MB.',
    params: { type: 'object', properties: { url: { type: 'string' }, max_chars: { type: 'number' } }, required: ['url'] }
  },
  [NAMES.TO_TABLE]: {
    description: 'Convert CSV, TSV or JSON data into a markdown table.',
    params: { type: 'object', properties: { data: { type: 'string' }, format: { type: 'string', enum: ['auto', 'csv', 'tsv', 'json'] }, max_rows: { type: 'number' } }, required: ['data'] }
  },
  [NAMES.JSON_TOOL]: {
    description: 'Validate, pretty-print or query JSON. Query paths like a.b[0].c.',
    params: { type: 'object', properties: { action: { type: 'string', enum: ['validate', 'format', 'query'] }, json: { type: 'string' }, path: { type: 'string' } }, required: ['action', 'json'] }
  },
  [NAMES.CLIPBOARD]: {
    description: 'Read from or write to the system clipboard. Reading requires explicit approval.',
    params: { type: 'object', properties: { action: { type: 'string', enum: ['read', 'write'] }, text: { type: 'string' } }, required: ['action'] }
  },
  [NAMES.NOTIFY]: {
    description: 'Show a desktop notification.',
    params: { type: 'object', properties: { title: { type: 'string' }, body: { type: 'string' } }, required: ['title', 'body'] }
  },
  [NAMES.TODO_LIST]: {
    description: 'Maintain a persistent to-do list for the current conversation (list/add/complete/clear).',
    params: { type: 'object', properties: { action: { type: 'string', enum: ['list', 'add', 'complete', 'clear'] }, text: { type: 'string' }, id: { type: 'string' } }, required: ['action'] }
  },
  [NAMES.WEATHER]: {
    description: 'Current weather and a short forecast (Open-Meteo, no key).',
    params: { type: 'object', properties: { latitude: { type: 'number' }, longitude: { type: 'number' }, location: { type: 'string' } }, required: ['latitude', 'longitude'] }
  },
  [NAMES.WIKIPEDIA]: {
    description: 'Fetch the summary of a Wikipedia article.',
    params: { type: 'object', properties: { title: { type: 'string' }, lang: { type: 'string' } }, required: ['title'] }
  },
  [NAMES.PLOT_CHART]: {
    description: 'Render a line or bar chart from data as inline SVG plus a trend summary.',
    params: { type: 'object', properties: { type: { type: 'string', enum: ['line', 'bar'] }, labels: { type: 'array', items: { type: 'string' } }, values: { type: 'array', items: { type: 'number' } }, title: { type: 'string' } }, required: ['type', 'labels', 'values'] }
  }
};

export function buildToolDefinitions() {
  const cfg = config();
  const defs = [];
  for (const [name, schema] of Object.entries(TOOL_SCHEMAS)) {
    if (cfg[CONFIG_KEYS[name]] !== false) {
      defs.push({ type: 'function', function: { name, description: schema.description, parameters: schema.params } });
    }
  }
  return defs;
}

/* ---- executors ---- */

function parseArgs(raw) {
  try {
    const parsed = JSON.parse(raw || '{}');
    return parsed && typeof parsed === 'object' ? parsed : {};
  } catch {
    return {};
  }
}

function formatNumber(value) {
  if (!Number.isFinite(value)) return String(value);
  const rounded = parseFloat(value.toPrecision(12));
  return Number.isInteger(rounded) && Math.abs(rounded) < 1e21 ? String(rounded) : String(rounded);
}

class CalcParser {
  constructor(expr) {
    const re = /(\d+\.?\d*(?:[eE][+-]?\d+)?|[a-zA-Z]+|[-+*/^%()!,.])/g;
    this.tokens = expr.match(re) ?? [];
    this.pos = 0;
  }
  peek() { return this.tokens[this.pos] ?? ''; }
  next() { return this.tokens[this.pos++] ?? ''; }
  parse() {
    const v = this.addSub();
    if (this.pos < this.tokens.length) throw new Error(`Unexpected token "${this.peek()}"`);
    return v;
  }
  addSub() {
    let v = this.mulDiv();
    for (;;) {
      const op = this.peek();
      if (op === '+' || op === '-') { this.next(); const r = this.mulDiv(); v = op === '+' ? v + r : v - r; }
      else return v;
    }
  }
  mulDiv() {
    let v = this.pow();
    for (;;) {
      const op = this.peek();
      if (op === '*' || op === '/' || op === '%') {
        this.next(); const r = this.pow();
        if (op === '*') v *= r;
        else if (op === '/') { if (r === 0) throw new Error('Division by zero'); v /= r; }
        else { if (r === 0) throw new Error('Modulo by zero'); v %= r; }
      } else return v;
    }
  }
  pow() {
    const base = this.unary();
    if (this.peek() === '^') { this.next(); return Math.pow(base, this.pow()); }
    return base;
  }
  unary() {
    const tok = this.peek();
    if (tok === '-') { this.next(); return -this.unary(); }
    if (tok === '+') { this.next(); return this.unary(); }
    const v = this.primary();
    if (this.peek() === '!') {
      this.next();
      if (!Number.isInteger(v) || v < 0) throw new Error('Factorial needs a non-negative integer');
      let r = 1;
      for (let i = 2; i <= v; i++) r *= i;
      return r;
    }
    return v;
  }
  primary() {
    const tok = this.next();
    if (tok === '(') { const v = this.addSub(); if (this.next() !== ')') throw new Error('Missing closing parenthesis'); return v; }
    if (tok === 'pi') return Math.PI;
    if (tok === 'e') return Math.E;
    const num = Number(tok);
    if (!Number.isNaN(num)) return num;
    const args = [];
    if (this.peek() === '(') {
      this.next();
      if (this.peek() !== ')') {
        args.push(this.addSub());
        while (this.peek() === ',') { this.next(); args.push(this.addSub()); }
      }
      this.next();
    }
    return this.fn(tok, args);
  }
  fn(name, args) {
    const n = args[0];
    switch (name) {
      case 'sqrt': return Math.sqrt(n); case 'abs': return Math.abs(n);
      case 'sin': return Math.sin(n); case 'cos': return Math.cos(n); case 'tan': return Math.tan(n);
      case 'asin': return Math.asin(n); case 'acos': return Math.acos(n); case 'atan': return Math.atan(n);
      case 'ln': return Math.log(n); case 'log10': return Math.log10(n); case 'exp': return Math.exp(n);
      case 'pow': return Math.pow(n, args[1]); case 'round': return Math.round(n);
      case 'floor': return Math.floor(n); case 'ceil': return Math.ceil(n);
      case 'min': return Math.min(...args); case 'max': return Math.max(...args);
      default: throw new Error(`Unknown function "${name}"`);
    }
  }
}

async function fetchUrl(url, maxChars) {
  if (!/^https?:\/\//i.test(url)) throw new Error('Only http(s) URLs are supported');
  const response = await fetch(url, { signal: AbortSignal.timeout(15000) });
  if (!response.ok) throw new Error(`HTTP ${response.status}`);
  const contentType = response.headers.get('content-type') ?? '';
  const text = await response.text();
  if (text.length > 1_000_000) throw new Error('Response exceeds the 1 MB cap');
  if (contentType.includes('text/html')) {
    const doc = new DOMParser().parseFromString(text, 'text/html');
    doc.querySelectorAll('script, style, noscript, svg, nav, footer, header').forEach((el) => el.remove());
    return (doc.body?.textContent ?? '').replace(/\s+/g, ' ').trim().slice(0, maxChars);
  }
  return text.replace(/\s+/g, ' ').trim().slice(0, maxChars);
}

function toTable(data, format, maxRows) {
  let rows = [];
  let header = null;
  if (format === 'json' || (format === 'auto' && data.trim().startsWith('['))) {
    const parsed = JSON.parse(data);
    if (!Array.isArray(parsed)) throw new Error('Expected a JSON array');
    const items = parsed.slice(0, maxRows);
    header = items.length ? Object.keys(items[0] ?? {}) : ['value'];
    rows = items.map((item) => header.map((h) => String(item?.[h] ?? '')));
  } else {
    const delimiter = format === 'tsv' ? '\t' : format === 'csv' ? ',' : data.includes('\t') ? '\t' : ',';
    const lines = data.trim().split(/\r?\n/).filter((l) => l.trim()).slice(0, maxRows + 1);
    rows = lines.map((line) => line.split(delimiter).map((c) => c.trim()));
    header = rows[0];
    rows = rows.slice(1);
  }
  const widths = header.map((_, col) => Math.max(...[header, ...rows].map((r) => (r[col]?.length ?? 0)), 3));
  const fmt = (cells) => '| ' + cells.map((c, i) => (c ?? '').padEnd(widths[i])).join(' | ') + ' |';
  const sep = '| ' + widths.map((w) => '-'.repeat(w)).join(' | ') + ' |';
  return [fmt(header), sep, ...rows.map(fmt)].join('\n');
}

function jsonQuery(value, path) {
  if (!path) return value;
  const parts = path.match(/[^.\[\]]+|\d+/g) ?? [];
  let current = value;
  for (const part of parts) {
    if (current === null || current === undefined) return undefined;
    if (Array.isArray(current)) current = current[Number(part)];
    else if (typeof current === 'object') current = current[part];
    else return undefined;
  }
  return current;
}

async function clipboardAction(action, text) {
  if (action === 'read') return (await navigator.clipboard.readText()) || '(clipboard is empty)';
  if (action === 'write') {
    if (!text) throw new Error('No text provided to write');
    await navigator.clipboard.writeText(text);
    return 'Copied to clipboard';
  }
  throw new Error(`Unknown action "${action}"`);
}

async function notify(title, body) {
  if (!('Notification' in window)) throw new Error('Notifications are not supported');
  if (Notification.permission === 'default') {
    const p = await Notification.requestPermission();
    if (p !== 'granted') throw new Error('Notification permission denied');
  }
  new Notification(title, { body });
  return `Notification shown: ${title}`;
}

const TODO_KEY = 'LlamaUi.todos';
function todoAction(conversationId, action, text, id) {
  let all = {};
  try { all = JSON.parse(localStorage.getItem(TODO_KEY) ?? '{}'); } catch { /* ignore */ }
  const todos = all[conversationId] ?? [];
  switch (action) {
    case 'list':
      return todos.length ? todos.map((t, i) => `${i + 1}. [${t.done ? 'x' : ' '}] ${t.text}${t.done ? ' (done)' : ''}`).join('\n') : 'No tasks yet.';
    case 'add': {
      if (!text?.trim()) throw new Error('Task text required');
      todos.push({ id: crypto.randomUUID(), text: text.trim(), done: false });
      all[conversationId] = todos;
      localStorage.setItem(TODO_KEY, JSON.stringify(all));
      return `Task added: ${text.trim()}`;
    }
    case 'complete': {
      const todo = todos.find((t) => t.id === id);
      if (!todo) throw new Error(`Unknown task id "${id}"`);
      todo.done = true;
      all[conversationId] = todos;
      localStorage.setItem(TODO_KEY, JSON.stringify(all));
      return `Completed: ${todo.text}`;
    }
    case 'clear':
      delete all[conversationId];
      localStorage.setItem(TODO_KEY, JSON.stringify(all));
      return 'Task list cleared';
    default:
      throw new Error(`Unknown action "${action}"`);
  }
}

async function weather(lat, lon, location) {
  const params = new URLSearchParams({
    latitude: String(lat), longitude: String(lon),
    current: 'temperature_2m,weather_code,wind_speed_10m',
    daily: 'weather_code,temperature_2m_max,temperature_2m_min', timezone: 'auto', forecast_days: '3'
  });
  const response = await fetch(`https://api.open-meteo.com/v1/forecast?${params}`, { signal: AbortSignal.timeout(15000) });
  if (!response.ok) throw new Error(`Weather API HTTP ${response.status}`);
  const data = await response.json();
  const current = data.current ?? {};
  const daily = data.daily ?? {};
  const label = location ? `${location} (${lat.toFixed(2)}, ${lon.toFixed(2)})` : `${lat.toFixed(2)}, ${lon.toFixed(2)}`;
  const lines = [`Weather for ${label}:`, `Now: ${current.temperature_2m ?? 'n/a'}°C, wind ${current.wind_speed_10m ?? 'n/a'} km/h`];
  for (let i = 0; i < (daily.time ?? []).length && i < 3; i++) {
    lines.push(`${daily.time[i]}: max ${daily.temperature_2m_max?.[i] ?? 'n/a'}°C / min ${daily.temperature_2m_min?.[i] ?? 'n/a'}°C`);
  }
  return lines.join('\n');
}

async function wikipedia(title, lang) {
  const response = await fetch(`https://${lang}.wikipedia.org/api/rest_v1/page/summary/${encodeURIComponent(title)}`, { signal: AbortSignal.timeout(15000) });
  if (response.status === 404) throw new Error(`No Wikipedia article "${title}"`);
  if (!response.ok) throw new Error(`Wikipedia HTTP ${response.status}`);
  const data = await response.json();
  return `Title: ${data.title ?? title}\n${(data.extract ?? '').slice(0, 4000)}`;
}

function esc(value) {
  return String(value).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function plotChart(type, labels, values, title) {
  const W = 560, H = 320, padL = 48, padR = 16, padT = 36, padB = 44;
  const max = Math.max(...values, 0), min = Math.min(...values, 0), range = max - min || 1;
  const plotW = W - padL - padR, plotH = H - padT - padB;
  const x = (i) => padL + (labels.length === 1 ? plotW / 2 : (plotW * i) / (labels.length - 1));
  const y = (v) => padT + plotH - ((v - min) / range) * plotH;
  const grid = Array.from({ length: 5 }, (_, i) => {
    const v = min + (range * i) / 4, gy = y(v);
    return `<line x1="${padL}" y1="${gy}" x2="${W - padR}" y2="${gy}" stroke="#e5e5e5"/><text x="${padL - 6}" y="${gy + 4}" text-anchor="end" font-size="11" fill="#888">${esc(formatNumber(v))}</text>`;
  }).join('');
  const body = type === 'bar'
    ? values.map((v, i) => {
        const bw = Math.max(4, (plotW / labels.length) * 0.6);
        const bx = padL + (plotW * i) / labels.length + (plotW / labels.length - bw) / 2;
        return `<rect x="${bx.toFixed(1)}" y="${Math.min(y(v), y(0)).toFixed(1)}" width="${bw.toFixed(1)}" height="${Math.max(1, Math.abs(y(v) - y(0))).toFixed(1)}" fill="#2563eb"/>`;
      }).join('')
    : values.map((v, i) => `${x(i).toFixed(1)},${y(v).toFixed(1)}`).join(' ');
  const labelsSvg = labels.map((l, i) => `<text x="${x(i).toFixed(1)}" y="${H - 12}" text-anchor="middle" font-size="11" fill="#888">${esc(String(l).slice(0, 14))}</text>`).join('');
  const titleSvg = title ? `<text x="${W / 2}" y="18" text-anchor="middle" font-size="14" font-weight="600" fill="#333">${esc(title)}</text>` : '';
  return ['```svg',
    `<svg xmlns="http://www.w3.org/2000/svg" width="${W}" height="${H}">${titleSvg}${grid}${type === 'line' ? `<polyline points="${body}" fill="none" stroke="#2563eb" stroke-width="2"/>` : body}${labelsSvg}</svg>`,
    '```', `Summary: ${values.length} points, min ${formatNumber(min)}, max ${formatNumber(max)}.`].join('\n');
}

export async function executeTool(name, rawArgs, context) {
  const args = parseArgs(rawArgs);
  try {
    switch (name) {
      case NAMES.CALCULATE:
        return { content: `= ${formatNumber(new CalcParser(String(args.expression ?? '')).parse())}`, isError: false };
      case NAMES.FETCH_URL:
        return { content: await fetchUrl(String(args.url ?? ''), Number(args.max_chars ?? 12000)), isError: false };
      case NAMES.TO_TABLE:
        return { content: toTable(String(args.data ?? ''), String(args.format ?? 'auto'), Number(args.max_rows ?? 100)), isError: false };
      case NAMES.JSON_TOOL: {
        let parsed;
        try { parsed = JSON.parse(String(args.json ?? '')); }
        catch (err) { throw new Error(`Invalid JSON: ${err.message}`); }
        if (args.action === 'validate') return { content: 'Valid JSON', isError: false };
        if (args.action === 'format') return { content: JSON.stringify(parsed, null, 2), isError: false };
        const result = jsonQuery(parsed, args.path ? String(args.path) : '');
        return { content: result === undefined ? 'No value at path' : JSON.stringify(result, null, 2), isError: false };
      }
      case NAMES.CLIPBOARD:
        return { content: await clipboardAction(String(args.action ?? ''), args.text ? String(args.text) : undefined), isError: false };
      case NAMES.NOTIFY:
        return { content: await notify(String(args.title ?? ''), String(args.body ?? '')), isError: false };
      case NAMES.TODO_LIST:
        return { content: todoAction(context.conversationId, String(args.action ?? ''), args.text ? String(args.text) : undefined, args.id ? String(args.id) : undefined), isError: false };
      case NAMES.WEATHER:
        return { content: await weather(Number(args.latitude), Number(args.longitude), args.location ? String(args.location) : undefined), isError: false };
      case NAMES.WIKIPEDIA:
        return { content: await wikipedia(String(args.title ?? ''), String(args.lang ?? 'en')), isError: false };
      case NAMES.PLOT_CHART:
        return { content: plotChart(String(args.type ?? 'line'), Array.isArray(args.labels) ? args.labels.map(String) : [], Array.isArray(args.values) ? args.values.map(Number) : [], args.title ? String(args.title) : undefined), isError: false };
      default:
        return { content: `Unknown built-in tool "${name}"`, isError: true };
    }
  } catch (err) {
    return { content: err instanceof Error ? err.message : String(err), isError: true };
  }
}

/** Tool display label for permission prompts. */
export function toolLabel(name) {
  return name;
}

/** The agentic loop: send with tools, execute calls, loop until done. */
export async function runAgenticLoop({ messages, options = {}, conversationId, onAssistantContent, onToolResult, signal }) {
  const tools = buildToolDefinitions();
  let history = [...messages];
  const maxTurns = 8;

  for (let turn = 0; turn < maxTurns; turn++) {
    const result = await collectStream(
      streamChatCompletion(history, {
        ...options,
        ...(tools.length ? { extra: { tools, tool_choice: 'auto' } } : {})
      }),
      signal,
      (content) => onAssistantContent?.(content)
    );

    if (result.content) onAssistantContent?.(result.content);
    if (!result.toolCalls?.length) return result;

    for (const tc of result.toolCalls) {
      if (signal?.aborted) return result;
      const name = tc.function?.name;
      if (!name) continue;

      const key = `frontend:${name}`;
      const allowed =
        permissions.hasTool(key) ||
        (await permissions.verify({
          title: t('Tool permission'),
          description: t('Allow use of {tool}?').replace('{tool}', name),
          confirmText: t('Allow once'),
          cancelText: t('Deny')
        }));
      if (!allowed) {
        log.warn('LLMUI-TL-002', 'tools: permission denied', name);
        history.push({ role: 'tool', tool_call_id: tc.id, content: 'Tool execution was denied by the user.' });
        continue;
      }

      let exec;
      try {
        exec = await executeTool(name, tc.function?.arguments ?? '', { conversationId });
      } catch (err) {
        exec = { content: err?.message ?? String(err), isError: true };
      }
      onToolResult?.(name, exec.content, exec.isError);
      history.push({ role: 'tool', tool_call_id: tc.id, content: exec.content });
      history.push({ role: 'assistant', content: null, tool_calls: [tc] });
    }
  }
  log.warn('LLMUI-TL-000', 'tools: agentic loop exceeded max turns');
  return { content: '', reasoning: '', toolCalls: [] };
}

/** Consume the kernel SSE generator into { content, reasoning, toolCalls }. */
async function collectStream(generator, signal, onChunk) {
  const result = { content: '', reasoning: '', toolCalls: [] };
  for await (const { delta, finish, reasoning, toolCalls } of generator) {
    if (signal?.aborted) return result;
    if (delta) result.content += delta;
    if (reasoning) result.reasoning += reasoning;
    if (toolCalls?.length) result.toolCalls = mergeToolCallDeltas(result.toolCalls, toolCalls);
    if (delta) onChunk?.(delta);
  }
  return result;
}

function mergeToolCallDeltas(existing, incoming) {
  const map = new Map(existing.map((tc) => [tc.index, tc]));
  for (const tc of incoming) {
    const current =
      map.get(tc.index) ?? { index: tc.index, id: '', type: 'function', function: { name: '', arguments: '' } };
    current.id = tc.id || current.id;
    current.function.name += tc.function?.name ?? '';
    current.function.arguments += tc.function?.arguments ?? '';
    map.set(tc.index, current);
  }
  return [...map.values()];
}
