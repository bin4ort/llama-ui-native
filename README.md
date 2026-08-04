# Llama UI Native

A native GTK desktop application wrapping the llama.cpp Web UI in a WebKitGTK window with a built-in HTTP server.

## Overview

- **Native wrapper**: C + GTK 3 + WebKitGTK 4.1
- **Frontend**: SvelteKit 5 static build served by the embedded minimal HTTP server (thread-per-connection, no third-party server library)
- **Backend**: Proxies `/v1/chat/completions` to a local llama.cpp server
- **Ports**: UI on `localhost:8765`, llama.cpp server expected on `localhost:8080`

## Project Structure

```
├── main.c / server.c     # GTK window + minimal HTTP server + API proxy
├── src/                  # SvelteKit source (edit here, then build)
├── frontend/v2/          # Compiled static output (served by C server)
│   ├── index.html
│   ├── _app/             # Bundled JS/CSS
│   ├── lang/             # Translation JSON files (12 languages)
│   └── i18n.js           # DOM-walking i18n fallback
├── scripts/              # Vite build plugins
├── ISSUES.md             # Known bugs & errors tracker
├── install.sh            # Build + install
└── launch.sh             # Launcher (sets LD_LIBRARY_PATH)
```

## Building

### Prerequisites

**Arch:**
```bash
sudo pacman -S gcc gtk3 webkit2gtk-4.1 curl pkgconf nodejs npm
```

**Debian/Ubuntu:**
```bash
sudo apt install gcc libgtk-3-dev libwebkit2gtk-4.1-dev libcurl4-openssl-dev pkg-config nodejs npm
```

### Build

```bash
# Frontend
npm install
npm run build
# Output goes to dist/ — already copied to frontend/v2/

# C binary
./install.sh
# Or: gcc -o llama-ui-native main.c server.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -lpthread -lm -lcurl -Wall
```

### Run

```bash
./launch.sh
```

## Adding a Language

1. Copy `frontend/v2/lang/en.json` to `frontend/v2/lang/{code}.json`
2. Translate all values (keep keys in English)
3. Add the language option in `src/lib/constants/settings-registry.ts` (line ~112)
4. Regenerate the embedded dict in `src/lib/stores/i18n.svelte.ts`:
   `node scripts/sync-embedded-dicts.mjs`
5. Add the language code in the `applyCode()` function of `src/lib/stores/i18n.svelte.ts`
6. Run `npm run build` and commit

## Translation Architecture

All UI strings are looked up by English key through a single mechanism:

- **`t("key")`** in templates and scripts — the canonical lookup
- **`tr["key"]` / `tr.Settings` / `tr.dict["key"]`** — all equivalent; every key of
  the active language dict is mirrored onto `tr` itself by `applyDict()`
- **Source of truth**: `frontend/v2/lang/*.json`. Edit these files, then run
  `node scripts/sync-embedded-dicts.mjs` to regenerate the inline `*_FULL` dicts
  in `src/lib/stores/i18n.svelte.ts` (embedded = no async fetch, no stale data)
- **Fallback**: `t("key")` returns the key itself when no translation exists

The `applyLang(code)` function switches languages at runtime (reactively via
`$state`); the user's choice is persisted in `localStorage['lang']`.

## Prompt Presets

Reusable system prompts (personas) that can be applied per conversation or
switched mid-conversation by the model via the `list_presets` /
`change_preset` tools.

- **Built-in library**: 5 presets ship with the app — Psychologist,
  Brainstorming Partner, Productivity Coach, Socratic Thinking Partner and
  Creative Writing Editor. They are seeded once on first run (marker:
  `systemPromptPresetsSeeded` in localStorage) and behave exactly like
  user-created presets: editable, starrable, deletable. Deleting them is
  permanent — the seed never runs twice. Dormant drafts (Language Tutor,
  Life Decision Advisor, Relationship Coach) live in
  `src/lib/constants/presets.ts` but are not shipped.
- **Preset wizard**: describe a role in plain language and the model drafts a
  detailed, structured system prompt (ROLE / METHOD / RESPONSE FORMAT / RULES,
  concrete techniques, answer formats). The wizard meta-prompt is
  `PRESET_WIZARD_META_PROMPT` in `src/lib/constants/presets.ts` — it forbids
  robotic filler and "see a specialist" cop-outs, and the result is always
  shown for review (never auto-saved).
- **Persona + context rows**: applying a preset sets the conversation's
  *persona* row; the per-conversation system message (*context* row) set via
  "Edit system message…" stacks underneath — both are sent, persona first.

## Notes

- Known bugs and code-quality issues are tracked in [ISSUES.md](ISSUES.md) —
  check it before merging feature work.
- The frontend is scheduled to be rewritten from SvelteKit/TypeScript to plain
  HTML/JS/CSS — see [REFACTOR-PLAN.md](REFACTOR-PLAN.md) (two-agent parallel
  plan, zero feature loss).
- Change `BACKEND_URL` in `server.h` to point to a different llama.cpp server
- The binary must run from the project directory (uses relative paths)
- `launch.sh` handles `LD_LIBRARY_PATH` for bundled libraries

## License

This project is free software: you can redistribute it and/or modify it under
the terms of the **GNU General Public License version 3** (or at your option,
any later version). See the [LICENSE](LICENSE) file for the full text.

GPL v3 was chosen over AGPL v3 because Llama UI Native is a desktop
application: users interact with it through the GTK/WebKit window, and the
embedded HTTP server binds to localhost only. The AGPL network clause targets
software served over a network (SaaS), which this project is not.

All dependencies have been audited for GPLv3 compatibility. Notably, the
embedded HTTP server is our own code (no vendored Mongoose — its GPL-2.0-only
license is incompatible with GPLv3, and switching the project to GPLv2 would
have clashed with the Apache-2.0 frontend deps).

Third-party components keep their own licenses:

- `src/lib/vendors/decimal.js/` — MIT
- npm dev dependencies — see `package-lock.json`
