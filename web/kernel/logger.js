/**
 * logger.js — leveled logger (console + in-memory ring buffer) and LlmUiError.
 *
 * Threshold: severity <= selected level is logged (0=error … 4=trace).
 * Ring buffer: last 500 entries, exposed for the Developer settings view.
 * Call sites never branch on level — the filter lives here.
 */
import { describe } from './error-codes.js';

export const LOG_LEVELS = {
  ERROR: 0,
  WARN: 1,
  INFO: 2,
  DEBUG: 3,
  TRACE: 4
};

const RING_SIZE = 500;
const ring = [];

let level = 2; // default: Info; set via settings-store on boot

export function setLogLevel(value) {
  level = Math.max(0, Math.min(4, Number(value) || 2));
}

export function getLogLevel() {
  return level;
}

const CONSOLE_FN = { 0: 'error', 1: 'warn', 2: 'info', 3: 'debug', 4: 'debug' };

function write(severity, code, message, detail, error) {
  if (severity > level) return;
  const ts = new Date().toISOString();
  const line = `${ts} | ${severity} | ${code} | ${message}`;
  const consoleFn = CONSOLE_FN[severity];
  if (error) console[consoleFn](line, detail ?? '', error);
  else if (detail !== undefined) console[consoleFn](line, detail);
  else console[consoleFn](line);

  ring.push({
    ts,
    level: severity,
    code,
    message,
    detail: detail === undefined ? null : detail
  });
  if (ring.length > RING_SIZE) ring.shift();
}

export const log = {
  error: (code, msg, detail, err) => write(0, code, msg, detail, err),
  warn: (code, msg, detail) => write(1, code, msg, detail),
  info: (code, msg, detail) => write(2, code, msg, detail),
  debug: (code, msg, detail) => write(3, code, msg, detail),
  trace: (code, msg, detail) => write(4, code, msg, detail)
};

/** Snapshot of the ring buffer (plain lines for "Copy debug log"). */
export function getRingBuffer() {
  return ring.map((e) => `${e.ts} | ${e.level} | ${e.code} | ${e.message}`).join('\n');
}

export function getRingEntries() {
  return [...ring];
}

/**
 * Error carrying a stable LLMUI code. UI renders `error.code` when present:
 * "Error LLMUI-API-012 — could not reach the model server".
 */
export class LlmUiError extends Error {
  constructor(code, message, detail) {
    const entry = describe(code);
    super(entry ? `${entry.msg}${detail !== undefined ? ` (${String(detail)})` : ''}` : message);
    this.name = 'LlmUiError';
    this.code = code;
    this.detail = detail;
    this.area = entry?.area ?? 'UNK';
  }
}
