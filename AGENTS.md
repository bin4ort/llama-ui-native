# AGENTS.md — Rules for agents working on this project

## Project structure
- `src/` — SvelteKit 5 source (edit here, then `npm run build`)
- `frontend/v2/` — Compiled static output (served by C server)
- `main.c` + `server.c` — GTK window + Mongoose HTTP server
- Single `npm run build` compiles the frontend

## Modification workflow
1. Edit source files in `src/`
2. Run `npm run build` from project root
3. Output goes to `dist/` — already configured to copy to `frontend/v2/`
4. Rebuild C: `gcc -o llama-ui-native main.c server.c mongoose.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -lpthread -lm -lcurl -Wall`
5. Never edit files in `frontend/v2/` directly (except `lang/*.json` and `i18n.js`)

## Frontend technology
- SvelteKit 5 with Svelte 5 runes ($state, $derived, $effect)
- Static adapter (`@sveltejs/adapter-static`)
- Single-page app mode (fallback: `index.html`)

## Translations (i18n)
- Core translations in `src/lib/stores/i18n.svelte.ts`
- `tr` is a $state object with named properties and `tr.dict` for dynamic strings
- Full dicts embedded inline (DE_FULL, RU_FULL, EN_FULL) — loaded synchronously
- Language files in `frontend/v2/lang/*.json` serve as reference/source for the inline dicts
- Adding a language: see README section "Adding a Language"
- UI templates use `{tr.dict["key"] || "key"}` for reactive translation

## Native wrapper (C code)
- Compile: `gcc -o llama-ui-native main.c server.c mongoose.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -lpthread -lm -lcurl -Wall`
- Dependencies: gtk+-3.0, webkit2gtk-4.1, libcurl
- Server port: 8765 (defined in `server.h`)
- Frontend served from `frontend/v2/` directory
- Proxies `/v1/chat/completions` to `http://localhost:8080`

## Git workflow
- All work in the project root — the repo is self-contained
- node_modules is symlinked from external tools but gitignored
- Commit meaningful changes, keep commits focused
