# AGENTS.md — Rules for agents working on this project

## Critical rules

1. **Build from source, never modify compiled output**
   - Source code lives at `/home/stry4ok/Development/Tools/llama.cpp/tools/ui/src/`
   - Build command: `npm run build` (run from `tools/ui/` directory)
   - After build, copy `dist/` contents to `frontend/v2/`
   - The `frontend/v2/` directory is the **compiled output** — never edit files in it directly (except `lang/*.json` translations and `i18n.js` which are post-build additions)

2. **Modification workflow**
   - Make changes in `tools/ui/src/**`
   - Run `npm run build` in `tools/ui/`
   - Copy `tools/ui/dist/*` into `frontend/v2/` (overwriting existing files)
   - Preserve `frontend/v2/lang/` (translation JSON files)
   - Preserve `frontend/v2/i18n.js` (DOM-walking fallback script)

3. **Frontend technology**
   - SvelteKit 5 with Svelte 5 runes ($state, $derived, $effect)
   - Tailwind CSS 4 via @tailwindcss/vite
   - Static adapter (`@sveltejs/adapter-static`)
   - Single-page app mode (fallback: `index.html`)

4. **Translations (i18n)**
   - Core translations use `$state` object `tr` in `src/lib/stores/i18n.svelte.ts`
   - Dynamic strings look up `tr.dict[key]` via the `t()` function
   - `tr.dict` is populated at runtime by fetching `/lang/{code}.json`
   - Hardcoded `$state` fields have inline DE/RU translations in the same file
   - `lang/*.json` files provide translations for everything else
   - `i18n.js` in `frontend/v2/` is a DOM-walking fallback (MUST be loaded by `index.html` — add `<script src="i18n.js"></script>` if missing)

5. **Native wrapper (C code)**
   - Compile with: `gcc -o llama-ui-native main.c server.c mongoose.c $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) -lpthread -lm -lcurl -Wall`
   - Dependencies: `gtk+-3.0`, `webkit2gtk-4.1`, `libcurl`
   - Server port: 8765 (defined in `server.h`)
   - Frontend served from `frontend/v2/` directory
   - The C server proxies `/v1/chat/completions` to `http://localhost:8080` (llama.cpp backend)
   - Also serves `/health`, `/api/network-info`, `/v1/models`, `/props`, `/slots` endpoints

6. **Configuration**
   - Backend URL: `server.h` → `BACKEND_URL` (default: `http://localhost:8080`)
   - Port: `server.h` → `BACKEND_PORT` (default: `8765`)
   - Version: `server.h` → `VERSION` and `BUILD`

7. **Adding a new language**
   - Create `frontend/v2/lang/{code}.json` (copy from `en.json` and translate all values)
   - Add hardcoded translations to `src/lib/stores/i18n.svelte.ts` for the $state fields
   - All `t()` calls in `.svelte` files will automatically pick up translations from the JSON dict
