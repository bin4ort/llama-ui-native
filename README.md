# Llama UI Native

A native GTK desktop application wrapping the llama.cpp Web UI in a WebKitGTK window with a built-in HTTP server.

## Overview

- **Native wrapper**: C + GTK 3 + WebKitGTK 4.1
- **Frontend**: SvelteKit 5 static build served by embedded Mongoose HTTP server
- **Backend**: Proxies `/v1/chat/completions` to a local llama.cpp server
- **Ports**: UI on `localhost:8765`, llama.cpp server expected on `localhost:8080`

## Project Structure

```
├── main.c / server.c     # GTK window + Mongoose HTTP server + API proxy
├── mongoose.c / .h       # Embedded web server
├── src/                  # SvelteKit source (edit here, then build)
├── frontend/v2/          # Compiled static output (served by C server)
│   ├── index.html
│   ├── _app/             # Bundled JS/CSS
│   ├── lang/             # Translation JSON files
│   └── i18n.js           # DOM-walking i18n fallback
├── scripts/              # Vite build plugins
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
# Or: gcc -o llama-ui-native main.c server.c mongoose.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -lpthread -lm -lcurl -Wall
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

## Notes

- Change `BACKEND_URL` in `server.h` to point to a different llama.cpp server
- The binary must run from the project directory (uses relative paths)
- `launch.sh` handles `LD_LIBRARY_PATH` for bundled libraries
