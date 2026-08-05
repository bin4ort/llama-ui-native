# AGENTS.md — Rules for agents working on this project

## Project structure
- `web/` — **vanilla frontend source** (no framework, no TypeScript; plain
  ES modules). Edit here, then `npm run build:web`
  - `web/kernel/` — shared foundation, **frozen** (router, stores, i18n,
    theme, settings-store, db, presets, permissions, api, error-codes,
    logger, modal, toast). Changes are contract changes — log them in
    REFACTOR-PLAN.md §4 changelog
  - `web/app/` — **Agent A tree** (shell, chat, streaming, markdown,
    chatbar, search) — do not edit as Agent B
  - `web/settings/` — **Agent B tree** (settings sections, MCP, presets,
    dialogs, PWA) — do not edit as Agent A
- `frontend/v3/` — compiled vanilla output (served by C server;
  `cp -a dist-web/. frontend/v3/` after `npm run build:web`)
- `frontend/v2/` + `src/` — **legacy SvelteKit** output/source, kept as
  migration fallback (v0.5.0 early alpha; delete after the parity gate)
- `main.c` + `server.c` — GTK window + self-contained HTTP server
- `ISSUES.md` — bug tracker (check before merging); `REFACTOR-PLAN.md` —
  migration plan + contract-changes log

## Modification workflow
1. Edit source files in `web/` (only your owned tree + frozen-kernel contract
   changes)
2. `npm run build:web` from project root (esbuild + Tailwind CLI + i18n dict
   generation from `frontend/v2/lang/*.json`)
3. Copy to the served dir: `cp -a dist-web/. frontend/v3/`
4. Rebuild C: `gcc -o llama-ui-native main.c server.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -ljxl -lpthread -lm -lcurl -Wall`
5. Never edit files in `frontend/v3/` or `dist-web/` directly

## Frontend technology
- Plain HTML + vanilla ES modules (esbuild bundles, Tailwind CSS v4 for
  utilities, kernel tokens for theming)
- SPA with hash router (`#/`, `#/chat/{id}`, `#/search`, `#/mcp-servers`,
  `#/settings/{section}`, `#/settings/presets`)
- Reactive state via `web/kernel/store.js` (pub/sub); i18n via `t()` with
  embedded dicts (generated into `web/kernel/dicts.generated.js`)

## Error codes & logging
- Every failure site logs a stable code `LLMUI-<AREA>-<NNN>` — registry in
  `web/kernel/error-codes.js` (frontend) and `web/kernel/error-codes.h`
  (native C server). **Append-only** — never reuse or renumber codes.
- Logger: `log.error/warn/info/debug/trace(code, msg, detail)`; threshold set
  by the Developer-settings log-level slider (`logLevel`, 0 Errors … 4 Trace)
  or `LLMUI_LOG_LEVEL` env (0/1/2) for the C server.
- `LlmUiError(code, msg, detail)` — throws carry the code; render
  "Error LLMUI-…" for the user.

## Translations (i18n)
- Source of truth: `frontend/v2/lang/*.json` (12 languages). The build
  regenerates `web/kernel/dicts.generated.js` from them automatically.
- `t('key')` with key-fallback; `tr.dict` for dynamic lookups.
- Adding a language: add `{code}.json` to `frontend/v2/lang/`, rebuild.

## Native wrapper (C code)
- Compile: `gcc -o llama-ui-native main.c server.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -ljxl -lpthread -lm -lcurl -Wall`
- Dependencies: gtk+-3.0, webkit2gtk-4.1, libcurl
- Server port: 8765 (defined in `server.h`); `FRONTEND_DIR` = `frontend/v3`
- `server.c` is a self-contained minimal HTTP server (thread-per-connection)
- Logs `[LLMUI-SRV-NNN]` stderr lines; `LLMUI_LOG_LEVEL` env filter (0/1/2)
- Proxies `/v1/chat/completions` to `http://localhost:8080`

## Versioning convention
- Bump per shipped batch (0.5.0 early alpha = vanilla frontend swap). Bump
  `VERSION`/`BUILD` in `server.h` (source of truth — feeds /health and
  build.json), mirror the fallback in `web/index.js` and `web/build.mjs`
  defaults, set `package.json` version, add a CHANGELOG entry, rebuild the
  frontend + C binary.

## Git workflow
- All work in the project root — the repo is self-contained
- node_modules is symlinked from external tools but gitignored
- Commit meaningful changes, keep commits focused
- Two agents work in parallel: Agent A owns `web/app/**`, Agent B owns
  `web/settings/**`. Never stage/commit the other agent's uncommitted files.
