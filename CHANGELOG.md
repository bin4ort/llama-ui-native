# Changelog

All notable changes to Llama UI Native are documented here.
Version is kept in `src/lib/constants/app.ts` (mirrored in `server.h` and `package.json`).

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
