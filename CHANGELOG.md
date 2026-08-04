# Changelog

All notable changes to Llama UI Native are documented here.
Version is kept in `src/lib/constants/app.ts` (mirrored in `server.h` and `package.json`).

## [0.4.2] - 2026-08-03

### Prompt presets (system prompt presets)

- **Preset library** — reusable system prompt presets (personas), stored in
  settings and included in export/import; favorites (max 5) surface in the
  chat-bar quick picker
- **Settings**: "System Message" renamed to **"Default system prompt"**; new
  collapsible **"Custom prompt presets"** manager (create / edit / star /
  delete)
- **Chat bar**: persona button with Default + favorite presets + "All
  presets…" full picker dialog (search, preview, select, manage favorites)
- **Wizard**: describe a personality in plain language → the model drafts the
  system prompt → review and save (never auto-saves)
- **Mid-conversation switching**: applying a preset updates the conversation's
  system message from the next message; agentic `list_presets` /
  `change_preset` tools (off by default, per-call permission) let the model
  switch personas mid-flow
- Work done on branch `feature/personality-presets`

## [0.4.3] - 2026-08-04

### Built-in tools

- **10 built-in day-to-day tools** (calculate, fetch_url, to_table, json_tool,
  clipboard, notify, todo_list, weather, wikipedia, plot_chart) implemented as
  frontend tools executed in the agentic loop, managed in the Tools settings
  tab with per-tool search and enable/disable
- **"Always allow" permissions** are staged and only applied on Save; new
  grants open a modular safety-verification dialog (cancelled grants are
  discarded); dialog is reusable (future: conversation vault)
- **Custom prompt presets** moved into the General tab below the Default
  system prompt, restyled like the tools menu (search, column header, rows),
  with hover/click description access
- Tooltips: fixed native-title suppression (now the app tooltip with a 300ms
  delay) and width-filling text (removed text-wrap balancing)

## Unreleased

### Licensing

- Project relicensed from MIT to **GNU GPL v3 (or later)** — see `LICENSE`,
  source headers in `main.c` / `server.c`, and the README "License" section
- `package.json` now declares `"license": "GPL-3.0-or-later"`
- **License audit**: every dependency verified GPLv3-compatible. The one
  exception — the vendored **Mongoose** HTTP server (GPL-2.0-only) — was
  removed and replaced with a self-written minimal HTTP server in `server.c`
  (thread-per-connection: static file serving, JSON endpoints, SSE proxy;
  ~600 lines, no third-party server code)
- Swapping to GPL-2.0(-or-later) instead was considered and rejected: the
  Apache-2.0 frontend deps (dexie, pdfjs-dist) are GPLv2-incompatible, so
  only the Mongoose side could be fixed

### Prompt wizard & built-in presets

- **Wizard overhaul**: the meta-prompt now produces detailed, professional
  system prompts (150–400 words, structured ROLE / METHOD / RESPONSE FORMAT /
  RULES sections, real field techniques, concrete answer formats), with a
  few-shot example for JSON reliability on small local models; wizard call
  uses `max_tokens: 1200`, `temperature: 0.4`
- Generated personas must act as the requested expert — no "see a specialist"
  cop-outs, no robotic filler, no invented studies
- **5 built-in presets** seeded on first run (one-time marker; existing
  libraries and later deletions are never overwritten): Psychologist,
  Brainstorming Partner, Productivity Coach, Socratic Thinking Partner,
  Creative Writing Editor — all non-programming, each with concrete methods,
  session formats and anti-pattern rules
- 3 further drafts (Language Tutor, Life Decision Advisor, Relationship
  Coach) authored in `src/lib/constants/presets.ts` but **not shipped** —
  dormant candidates for a future opt-in library

### Languages & fonts

- **12 languages**: added 中文 (zh), 日本語 (ja), 한국어 (ko), Português (pt),
  Italiano (it), Türkçe (tr), Polski (pl) — full 617-key dictionaries each
- **Bundled fonts** so every language renders correctly without system fonts:
  Noto Sans (Latin/Cyrillic/Greek/Vietnamese — covers de/es/fr/it/pt/tr/pl/ru)
  and Noto Sans SC/JP/KR (CJK); per-language font stack ordering via
  `html:lang`
- **Language dropdown** opens as a fixed 5-option scrollable window and no
  longer scrolls the page when the wheel hits the top/bottom of the list

### Removed

- Build version tracker (`b0000`) in the bottom-right corner — display,
  setting, store, plugin and `build.json` are gone

## [0.4.0] - 2026-08-03

### Translation system (complete)

- **Unified translation mechanism**: every key of the active language dict is
  mirrored onto the `tr` store, so `t("key")`, `tr["key"]` and `tr.dict["key"]`
  all resolve consistently. Removed the async `/lang/*.json` fetch that could
  clobber the applied dict (stale/race conditions).
- **Embedded dicts are the source of truth**: the 5 inline `*_FULL` dicts in
  `src/lib/stores/i18n.svelte.ts` are regenerated from `frontend/v2/lang/*.json`
  via `scripts/sync-embedded-dicts.mjs` — no drift between languages.
- **Added Spanish (ES) and French (FR)**, completed German (DE) and Russian (RU),
  fixed French dict missing 57 keys.
- **Full UI coverage**: settings page titles, settings sidebar, import/export tab
  (sections, dialogs, toasts, alerts), MCP servers page, chat attach menu
  (dropdown + sheet + disabled tooltips), file-type labels, plural forms
  (`resources`, `templates`, `servers`, `conversations`), `and {n} more`
  interpolation, reset dialog, localStorage note, export warning.
- ~25 new translation keys added across all 5 languages.
- `tr` typed with an index signature — fixed 20 pre-existing TS errors.

### Version & build system

- `npm run build` fixed (the dead `build-pwa-assets` step referencing removed
  config files was dropped; build is now a single `vite build`).
- Version centralized in `src/lib/constants/app.ts` (`APP_VERSION`, `APP_BUILD`),
  mirrored to `server.h` and `package.json` — the settings footer now renders
  from these constants instead of a hardcoded string.
- Version bumped to **0.4.0**.

### Docs & housekeeping

- `CHANGELOG.md` and `TODO.md` added.
- README updated: "Adding a Language" workflow and translation architecture
  section reflect the new single-mechanism design.

## [0.3.0] - 2026-08-02

Initial release of the native (GTK + WebKit) wrapper around the llama.cpp Web UI:

- Native window (`main.c`) + HTTP server on port 8765 (`server.c`, Mongoose),
  proxying `/v1/chat/completions` to `http://localhost:8080`
- SvelteKit 5 frontend (static SPA build served from `frontend/v2/`)
- Chat, settings (General / Display / Sampling / Penalties / Tools / Agentic /
  Developer / Import-Export), MCP client, tools, attachment upload,
  import/export of conversations and settings
- Languages: English, German, Russian (partial)
- Dark/light themes, PWA scaffolding (assets removed in debloat)
