# Llama UI Native

A native GTK desktop application that wraps the [llama.cpp Web UI](https://github.com/ggml-org/llama.cpp) in a WebKitGTK window with a built-in HTTP server for API proxying.

## Overview

- **Native wrapper**: C application using GTK 3 + WebKitGTK 4.1
- **Frontend**: SvelteKit 5 static build served by embedded Mongoose HTTP server
- **Backend proxy**: Proxies `/v1/chat/completions` to a local llama.cpp server
- **Port**: UI runs on `http://localhost:8765`, expects llama.cpp server on port 8080

## Project Structure

```
LlamaUI-Native/
├── main.c              # GTK window + WebKitWebView
├── server.c/.h         # Mongoose HTTP server + API proxy
├── mongoose.c/.h       # Mongoose embedded web server
├── frontend/v2/        # Compiled SvelteKit static output
│   ├── index.html      # Entry point
│   ├── _app/           # Bundled JS/CSS assets
│   ├── lang/           # Translation JSON files (en.json, de.json, ru.json)
│   └── i18n.js         # DOM-walking i18n fallback script
├── install.sh          # One-command build + install script
├── launch.sh           # Launcher (sets LD_LIBRARY_PATH)
└── scripts/            # Vite plugins (build info, splash screen, etc.)
```

The source code for the Web UI lives in a separate tree:
```
/home/stry4ok/Development/Tools/llama.cpp/tools/ui/
├── src/                # SvelteKit source
│   ├── routes/         # Page routes
│   ├── lib/
│   │   ├── components/ # UI components
│   │   └── stores/     # State stores (including i18n.svelte.ts)
│   └── app.html        # HTML template
├── static/             # Static assets (favicons, etc.)
├── scripts/            # Vite plugins
├── lang/               # (Translation files built into dist/ at build time)
└── package.json        # Dependencies and build scripts
```

## Building

### Prerequisites

**Arch Linux:**
```bash
sudo pacman -S gcc gtk3 webkit2gtk-4.1 curl pkgconf
```

**Debian/Ubuntu:**
```bash
sudo apt install gcc libgtk-3-dev libwebkit2gtk-4.1-dev libcurl4-openssl-dev pkg-config
```

### Build the frontend

```bash
cd /home/stry4ok/Development/Tools/llama.cpp/tools/ui
npm install
npm run build
```

### Copy frontend output

```bash
cp -r /home/stry4ok/Development/Tools/llama.cpp/tools/ui/dist/* \
      /home/stry4ok/Development/Projects/LlamaUI-Native/frontend/v2/
```

### Compile the native binary

```bash
cd /home/stry4ok/Development/Projects/LlamaUI-Native
./install.sh
```

Or manually:
```bash
gcc -o llama-ui-native main.c server.c mongoose.c \
    $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) \
    -lpthread -lm -lcurl -Wall
```

### Running

```bash
./launch.sh
# Or directly:
./llama-ui-native
```

## Adding Language Support

1. Copy the English translation file:
   ```bash
   cp frontend/v2/lang/en.json frontend/v2/lang/{code}.json
   ```

2. Translate all values in the new JSON file. Keep keys in English.

3. Add hardcoded `$state` field translations to `src/lib/stores/i18n.svelte.ts` in the source tree, then rebuild the frontend.

4. The language is selected at runtime via the Settings panel. The selection is stored in `localStorage` key `lang`.

## Architecture

### Translation System (i18n)

The app uses a two-layer translation system:

1. **SvelteKit $state** (`src/lib/stores/i18n.svelte.ts`):
   - Core UI strings use typed `$state` fields (e.g., `tr.Settings`, `tr.NewChat`)
   - Hardcoded translations for DE and RU inline
   - Dynamic strings use `tr.dict` populated from `/lang/{code}.json` at runtime
   - `t(key)` function looks up `tr.dict[key]` or returns the key as fallback

2. **DOM-walking fallback** (`frontend/v2/i18n.js`):
   - Background script that walks text nodes and replaces them using de.json
   - Handles strings that Svelte's reactive system missed
   - Polls `localStorage` for language changes every 300ms

### Server Architecture

- Mongoose HTTP server serves static files from `frontend/v2/`
- API-like endpoints: `/health`, `/api/network-info`, `/v1/models`, `/props`, `/slots`
- SSE proxy: `/v1/chat/completions` and `/completion` are proxied to the llama.cpp backend via libcurl
- CORS headers are set for all responses

### GTK Wrapper

- `main.c` creates a GTK Window with a fullscreen WebKitWebView
- External links are opened in the system browser
- Dark background (#141414) set via GTK CSS provider
- Developer extras enabled: Web Inspector available

## Notes

- The C server expects a llama.cpp server running on `http://localhost:8080`
- Change `BACKEND_URL` in `server.h` to use a different backend
- The native binary must run from the project directory (it uses relative paths)
- The `launch.sh` script handles setting `LD_LIBRARY_PATH` for bundled libraries
