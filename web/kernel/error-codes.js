/**
 * error-codes.js — LLMUI error-code registry (single source of truth).
 *
 * Format: LLMUI-<AREA>-<NNN>. Append-only: codes are never reused or
 * renumbered; a removed site leaves its code reserved with status 'retired'.
 * This file is FROZEN (kernel) — new codes are contract changes.
 * docs/ERROR-CODES.md is generated from this table at build time.
 */

export const AREAS = {
  SYS: 'app boot / lifecycle / version',
  CFG: 'settings store, prop sync, reset',
  DB: 'IndexedDB (Dexie) / conversations / migrations',
  API: 'HTTP client (models, props, slots, tools, MCP transport)',
  STR: 'streaming / SSE / chat completion transport',
  CHAT: 'chat store, message tree, branching, titles',
  MD: 'markdown pipeline (marked, LaTeX, mermaid, svg blocks)',
  IMG: 'image attachments (resize, EXIF, HEIC)',
  AUD: 'audio (recording, files, transcription)',
  PDF: 'PDF -> text extraction',
  MCP: 'MCP client, resources, prompts, sessions',
  TL: 'tools / agentic loop / permissions / verification',
  PRS: 'presets (library, seeding, wizard, change_preset tool)',
  SEC: 'sandbox, sanitization, redaction',
  PWA: 'service worker, manifest, updates',
  UI: 'generic UI failures (modal, toast, render)'
};

export const CODES = {
  /* SYS */
  'LLMUI-SYS-000': { area: 'SYS', msg: 'boot: entry script failed', status: 'active' },
  'LLMUI-SYS-001': { area: 'SYS', msg: 'boot: kernel initialization failed', status: 'active' },
  'LLMUI-SYS-002': { area: 'SYS', msg: 'router: no route handler matched', status: 'active' },
  'LLMUI-SYS-003': { area: 'SYS', msg: 'router: hash parse failed', status: 'active' },

  /* CFG */
  'LLMUI-CFG-000': { area: 'CFG', msg: 'settings: localStorage read/parse failed', status: 'active' },
  'LLMUI-CFG-001': { area: 'CFG', msg: 'settings: localStorage write failed', status: 'active' },
  'LLMUI-CFG-002': { area: 'CFG', msg: 'settings: server prop sync failed', status: 'active' },
  'LLMUI-CFG-003': { area: 'CFG', msg: 'settings: reset to defaults failed', status: 'active' },
  'LLMUI-CFG-004': { area: 'CFG', msg: 'settings: import rejected (bad payload)', status: 'active' },

  /* DB */
  'LLMUI-DB-000': { area: 'DB', msg: 'dexie: database open failed', status: 'active' },
  'LLMUI-DB-001': { area: 'DB', msg: 'dexie: conversation add failed', status: 'active' },
  'LLMUI-DB-002': { area: 'DB', msg: 'dexie: message add failed', status: 'active' },
  'LLMUI-DB-003': { area: 'DB', msg: 'dexie: update failed', status: 'active' },
  'LLMUI-DB-004': { area: 'DB', msg: 'dexie: delete failed', status: 'active' },
  'LLMUI-DB-005': { area: 'DB', msg: 'dexie: conversation tree build failed', status: 'active' },
  'LLMUI-DB-006': { area: 'DB', msg: 'migration: legacy key migration failed', status: 'active' },
  'LLMUI-DB-007': { area: 'DB', msg: 'export: conversations serialization failed', status: 'active' },
  'LLMUI-DB-008': { area: 'DB', msg: 'import: conversations parse failed', status: 'active' },

  /* API */
  'LLMUI-API-000': { area: 'API', msg: 'http: request failed (network)', status: 'active' },
  'LLMUI-API-001': { area: 'API', msg: 'http: non-2xx response', status: 'active' },
  'LLMUI-API-002': { area: 'API', msg: 'api: response parse failed', status: 'active' },
  'LLMUI-API-003': { area: 'API', msg: 'api: models list fetch failed', status: 'active' },
  'LLMUI-API-004': { area: 'API', msg: 'api: props fetch failed', status: 'active' },
  'LLMUI-API-005': { area: 'API', msg: 'api: slots fetch failed', status: 'active' },
  'LLMUI-API-006': { area: 'API', msg: 'api: tools list (/tools) fetch failed', status: 'active' },
  'LLMUI-API-007': { area: 'API', msg: 'api: tool execute failed', status: 'active' },
  'LLMUI-API-008': { area: 'API', msg: 'api: model load/unload failed', status: 'active' },
  'LLMUI-API-009': { area: 'API', msg: 'api: unauthorized (401/403)', status: 'active' },
  'LLMUI-API-010': { area: 'API', msg: 'cors-proxy: upstream request failed', status: 'active' },
  'LLMUI-API-011': { area: 'API', msg: 'http: response exceeded size cap', status: 'active' },
  'LLMUI-API-012': { area: 'API', msg: 'fetch_url: unsupported protocol', status: 'active' },

  /* STR */
  'LLMUI-STR-000': { area: 'STR', msg: 'stream: connection failed', status: 'active' },
  'LLMUI-STR-001': { area: 'STR', msg: 'stream: SSE frame parse failed', status: 'active' },
  'LLMUI-STR-002': { area: 'STR', msg: 'stream: aborted mid-chunk', status: 'active' },
  'LLMUI-STR-003': { area: 'STR', msg: 'stream: resume failed', status: 'active' },
  'LLMUI-STR-004': { area: 'STR', msg: 'stream: timeout', status: 'active' },
  'LLMUI-STR-005': { area: 'STR', msg: 'stream: server error event', status: 'active' },
  'LLMUI-STR-006': { area: 'STR', msg: 'stream: empty completion', status: 'active' },
  'LLMUI-STR-007': { area: 'STR', msg: 'stream: tool-call chunk aggregation failed', status: 'active' },
  'LLMUI-STR-008': { area: 'STR', msg: 'stream: completion id missing', status: 'active' },
  'LLMUI-STR-009': { area: 'STR', msg: 'stream: timings parse failed', status: 'active' },
  'LLMUI-STR-010': { area: 'STR', msg: 'stream: queue overflow (pending messages)', status: 'active' },

  /* CHAT */
  'LLMUI-CHAT-000': { area: 'CHAT', msg: 'chat: message tree insert failed', status: 'active' },
  'LLMUI-CHAT-001': { area: 'CHAT', msg: 'chat: branch/regenerate failed', status: 'active' },
  'LLMUI-CHAT-002': { area: 'CHAT', msg: 'chat: title generation failed', status: 'active' },
  'LLMUI-CHAT-003': { area: 'CHAT', msg: 'chat: system prompt apply failed', status: 'active' },
  'LLMUI-CHAT-004': { area: 'CHAT', msg: 'chat: pending message conflict', status: 'active' },
  'LLMUI-CHAT-005': { area: 'CHAT', msg: 'chat: conversation delete failed', status: 'active' },
  'LLMUI-CHAT-006': { area: 'CHAT', msg: 'chat: edit message failed', status: 'active' },
  'LLMUI-CHAT-007': { area: 'CHAT', msg: 'chat: pre-encode failed', status: 'active' },
  'LLMUI-CHAT-008': { area: 'CHAT', msg: 'chat: fork conversation failed', status: 'active' },

  /* MD */
  'LLMUI-MD-000': { area: 'MD', msg: 'markdown: marked parse failed', status: 'active' },
  'LLMUI-MD-001': { area: 'MD', msg: 'markdown: latex render failed', status: 'active' },
  'LLMUI-MD-002': { area: 'MD', msg: 'markdown: mermaid render failed', status: 'active' },
  'LLMUI-MD-003': { area: 'MD', msg: 'markdown: svg block sanitize rejected', status: 'active' },
  'LLMUI-MD-004': { area: 'MD', msg: 'markdown: dompurify sanitize failed', status: 'active' },
  'LLMUI-MD-005': { area: 'MD', msg: 'markdown: table restorer failed', status: 'active' },

  /* IMG */
  'LLMUI-IMG-000': { area: 'IMG', msg: 'image: decode failed', status: 'active' },
  'LLMUI-IMG-001': { area: 'IMG', msg: 'image: resize failed', status: 'active' },
  'LLMUI-IMG-002': { area: 'IMG', msg: 'image: EXIF orientation failed', status: 'active' },
  'LLMUI-IMG-003': { area: 'IMG', msg: 'image: HEIC decode failed', status: 'active' },

  /* AUD */
  'LLMUI-AUD-000': { area: 'AUD', msg: 'audio: mic access denied', status: 'active' },
  'LLMUI-AUD-001': { area: 'AUD', msg: 'audio: recording failed', status: 'active' },
  'LLMUI-AUD-002': { area: 'AUD', msg: 'audio: file decode failed', status: 'active' },
  'LLMUI-AUD-003': { area: 'AUD', msg: 'audio: transcription failed', status: 'active' },

  /* PDF */
  'LLMUI-PDF-000': { area: 'PDF', msg: 'pdf: worker load failed', status: 'active' },
  'LLMUI-PDF-001': { area: 'PDF', msg: 'pdf: document parse failed', status: 'active' },
  'LLMUI-PDF-002': { area: 'PDF', msg: 'pdf: text extraction failed', status: 'active' },

  /* MCP */
  'LLMUI-MCP-000': { area: 'MCP', msg: 'mcp: connect failed', status: 'active' },
  'LLMUI-MCP-001': { area: 'MCP', msg: 'mcp: initialize handshake failed', status: 'active' },
  'LLMUI-MCP-002': { area: 'MCP', msg: 'mcp: tools list failed', status: 'active' },
  'LLMUI-MCP-003': { area: 'MCP', msg: 'mcp: resources list failed', status: 'active' },
  'LLMUI-MCP-004': { area: 'MCP', msg: 'mcp: prompts list failed', status: 'active' },
  'LLMUI-MCP-005': { area: 'MCP', msg: 'mcp: tool call failed', status: 'active' },
  'LLMUI-MCP-006': { area: 'MCP', msg: 'mcp: resource read failed', status: 'active' },
  'LLMUI-MCP-007': { area: 'MCP', msg: 'mcp: prompt get failed', status: 'active' },

  /* TL */
  'LLMUI-TL-000': { area: 'TL', msg: 'tools: agentic loop exceeded max turns', status: 'active' },
  'LLMUI-TL-001': { area: 'TL', msg: 'tools: tool call parse failed', status: 'active' },
  'LLMUI-TL-002': { area: 'TL', msg: 'tools: permission decision missing', status: 'active' },
  'LLMUI-TL-003': { area: 'TL', msg: 'tools: verification cancelled/failed', status: 'active' },
  'LLMUI-TL-004': { area: 'TL', msg: 'tools: built-in executor failed', status: 'active' },
  'LLMUI-TL-005': { area: 'TL', msg: 'tools: sandbox execution failed', status: 'active' },
  'LLMUI-TL-006': { area: 'TL', msg: 'tools: steering message inject failed', status: 'active' },

  /* PRS */
  'LLMUI-PRS-000': { area: 'PRS', msg: 'presets: library parse failed', status: 'active' },
  'LLMUI-PRS-001': { area: 'PRS', msg: 'presets: wizard generation failed', status: 'active' },
  'LLMUI-PRS-002': { area: 'PRS', msg: 'presets: wizard JSON parse failed', status: 'active' },
  'LLMUI-PRS-003': { area: 'PRS', msg: 'presets: change_preset target not found', status: 'active' },
  'LLMUI-PRS-004': { area: 'PRS', msg: 'presets: seeding marker write failed', status: 'active' },

  /* SEC */
  'LLMUI-SEC-000': { area: 'SEC', msg: 'sandbox: harness build failed', status: 'active' },
  'LLMUI-SEC-001': { area: 'SEC', msg: 'sandbox: worker timeout', status: 'active' },
  'LLMUI-SEC-002': { area: 'SEC', msg: 'sandbox: output cap exceeded', status: 'active' },
  'LLMUI-SEC-003': { area: 'SEC', msg: 'redact: sensitive value leak detected', status: 'active' },

  /* PWA */
  'LLMUI-PWA-000': { area: 'PWA', msg: 'sw: install/activate failed', status: 'active' },
  'LLMUI-PWA-001': { area: 'PWA', msg: 'sw: precache failed', status: 'active' },
  'LLMUI-PWA-002': { area: 'PWA', msg: 'sw: update check failed', status: 'active' },

  /* UI */
  'LLMUI-UI-000': { area: 'UI', msg: 'ui: modal open failed', status: 'active' },
  'LLMUI-UI-001': { area: 'UI', msg: 'ui: toast failed', status: 'active' },
  'LLMUI-UI-002': { area: 'UI', msg: 'ui: render error boundary', status: 'active' },
  'LLMUI-UI-003': { area: 'UI', msg: 'ui: clipboard api failed', status: 'active' }
};

/** Resolve a code (or null for unknown). */
export function describe(code) {
  const entry = CODES[code];
  return entry ? { code, ...entry } : null;
}

/** Generate the markdown table for docs/ERROR-CODES.md. */
export function toMarkdown() {
  const lines = ['# LLMUI Error Codes', '', '| Code | Area | Message | Status |', '|---|---|---|---|'];
  for (const [code, entry] of Object.entries(CODES)) {
    lines.push(`| ${code} | ${entry.area} | ${entry.msg} | ${entry.status} |`);
  }
  return lines.join('\n') + '\n';
}
