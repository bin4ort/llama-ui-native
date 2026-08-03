# TODO

## Public roadmap

Ideas and features that matter to users. Priority is indicative.

- [ ] **Conversation search filters** — search by date range / model used
- [ ] **MCP server presets** — one-click config for common servers (filesystem, fetch, ...)
- [ ] **Custom prompt library** — store and reuse prompts per conversation
- [ ] **More languages** — follow "Adding a Language" in README (dicts are ready for it)
- [ ] **Automatic app updates** — check llama.cpp releases / notify in-app
- [ ] **Tray icon & background server** — run llama-server from the app
- [ ] **Per-conversation system message & model** — not just per-chat defaults
- [ ] **Token usage chart** — visualize context usage over time

## Internal / maintainers

Technical debt and things to watch. Tick items when done.

### Type safety
- [ ] Fix the 8 remaining `svelte-check` errors (baseline before translation work):
  - `SettingsChat.svelte` — `mobileHeader` implicit `any` (5 errors)
  - `SettingsChatFields.svelte` — implicit `any` params (`opt`, `o`)
  - `routes/settings/[[section]]/+page.svelte` — missing `getSectionHref` prop
  - `ChatMessageAssistantProcessingInfo.svelte` — line 15 type error

### Build & scripts
- [ ] `scripts/dev.sh` references `tools/ui/` paths from the llama.cpp layout —
      either fix for this repo or delete
- [ ] `scripts/git-hooks/` (pre-commit/pre-push) only act on `tools/ui/` paths —
      make them cover this repo (prettier + `npm run check`) or remove
- [ ] `scripts/favicon-colorize.ts`, `scripts/make-icons-circular.js`,
      `scripts/vite-plugin-splash-screen.ts` are PWA-era leftovers — verify
      they are still needed or delete
- [ ] `launch.sh` / `install.sh` — verify after version bump (no C recompile needed
      for JS changes, but binary should be rebuilt with new VERSION/BUILD from
      `server.h` before release)
- [ ] Consider wiring `scripts/sync-embedded-dicts.mjs` into the build so
      editing `frontend/v2/lang/*.json` never desyncs the bundle again

### i18n
- [ ] Prettier-check the regenerated inline dicts (they are single-quoted/JSON
      style, prettier may reformat on `npm run format`)
- [ ] Audit remaining untranslated surfaces: browser tab title
      (`Chat … - llama-ui`), native dialogs (GTK file pickers), version footer
      (intentionally static)
- [ ] Plural forms for Russian are simplified (genitive plural for counts) —
      proper 1/2-4/5+ forms would be more correct

### Testing
- [ ] Add a unit test for `applyDict`/`t()` (mirror + fallback + language switch)
- [ ] Run `npm run test:unit` once CI/Playwright setup is wired for this repo
