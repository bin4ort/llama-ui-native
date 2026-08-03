# Draft — Built-in Tools for the AI Chat

> Status: **PROPOSED — not implemented.** This is a planning document for tools
> that should ship built into Llama UI Native (no external MCP server needed).
> Everything here is a candidate; nothing is committed yet.

> **Day-to-day custom tools** (what to actually build, excluding llama.cpp's
> built-ins): see `docs/day-to-day-tools-draft.md`.

## Already covered by llama.cpp (just needs `--tools`)

The llama.cpp server ships built-in agent tools behind the `--tools` flag
(`--tools all`, or a comma-separated subset; `--agent` enables all + CORS
proxy). Security note from upstream: *do not enable in untrusted environments*;
it also restricts `--cors-origins` to localhost by default. The app already
fetches these from `/tools` and renders them (403 from `/tools` = server was
started without `--tools`).

| Available llama.cpp tool | Draft overlap |
| --- | --- |
| `read_file` | workspace FS |
| `write_file` / `edit_file` | workspace FS |
| `file_glob_search` | workspace FS (glob) |
| `grep_search` | workspace FS (grep) |
| `exec_shell_command` | sandboxed shell — **but un-sandboxed**: runs on the server host with full privileges; the app-side sandbox (no-network, cwd=workspace) is still needed |
| `get_datetime` | `get_current_time` — already solved upstream |
| `get_info` | system info |

**Consequence:** the draft below should NOT re-implement read/write/edit/glob/
grep/datetime as app tools. Instead the app should (a) document/forward the
`--tools` flags (later: launch the server itself), (b) add per-tool permission
gating for the server tools, and (c) only app-build what llama.cpp lacks.

## What exists today

| Tool / capability | Where it lives |
| --- | --- |
| `run_javascript` | App-built sandbox (Web Worker in opaque-origin iframe, hard timeout, `nerdamer` optional) |
| Python (Pyodide) | App-built, executes Python in markdown code blocks |
| Symbolic math (`nerdamer`) | App-built, optional preload into the JS sandbox |
| MCP client | Any external MCP server (filesystem, fetch, …) |
| `read_file` / `write_file` / `edit_file` / `glob` / `grep` / `exec_shell_command` | Rendered by the app, but provided by the **llama.cpp server** tool schemas, not by the app |

Existing building blocks we can reuse: tool-call parsers + renderers
(`parsers/`), the permission UI ("Allow once" / "Deny" / "More allow options"),
settings-gated tool toggles (like the sandbox settings), and the local
`/cors-proxy` endpoint for HTTP.

## Design principles

1. **Local-first and private.** Default tools run on-device, no cloud accounts.
   Anything network-bound must be opt-in and clearly labeled.
2. **Consent for anything outside the sandbox.** FS writes, shell, clipboard,
   notifications → per-call Allow once / Deny (UI already exists).
3. **Small, composable tools.** One JSON-schema tool per primitive; the model
   composes them.
4. **Deterministic tools need no model.** Calculator, date/time, table
   formatting are cheap and far more reliable than prompting.
5. **Native advantage.** The app wraps a llama-server, so model lifecycle
   (load/unload/slots) and desktop integration (notifications, clipboard,
   filesystem) are fair game — no browser-only limitations.

## Proposed tools

### Tier 1 — cheap, high-value, low risk

1. **Calculator** (`calculate`)
   - Arbitrary-precision arithmetic (Decimal/BigInt), expression parser.
   - The model stops doing arithmetic in prose; results are exact.
2. **Current date/time** — ✅ already solved: llama.cpp `get_datetime`
   (`--tools get_datetime`). Only app-side work: surface it in the tools UI.
3. **Workspace filesystem** — ✅ read/write/edit/glob/grep already exist as
   llama.cpp built-ins. Only app-side work: path-containment + workspace-root
   policy **is not enforced by the server** (it reads/writes anywhere the
   server user can) — decide whether the app restricts it or documents the
   server's trust boundary.
4. **Fetch URL** (`fetch_url`)
   - HTTP GET via the existing local `/cors-proxy`; response size cap
     (e.g. 1 MB), timeout, content-type sniffing, HTML→text extraction.
   - This is the foundation for everything web-related below.

### Tier 2 — strong additions, more surface area

5. **Web search** (`web_search`)
   - Wraps an optional search provider (SearXNG / DuckDuckGo / configured API
     key). Off by default; nothing sent anywhere until the user enables it.
6. **Mermaid / diagram validation** (`generate_mermaid`)
   - The app already bundles mermaid for rendering; a tool that validates +
     renders a diagram spec into SVG/PNG preview inside the chat.
7. **Task / todo tracker** (`todo_list`)
   - Persistent per-conversation todo list (add, complete, list, clear).
   - Lets the agent keep multi-step work state across turns instead of
     re-deriving it from the transcript.
8. **Data table viewer** (`to_table`)
   - CSV/TSV/JSON → formatted table preview (client-side only), plus markdown
     rendering hooks already present.
9. **Clipboard** (`read_clipboard`, `write_clipboard`)
   - Native wrapper → GTK clipboard. Read requires explicit user approval
     (privacy); write is safe and useful ("copy this result").
10. **Desktop notification** (`notify`)
    - Native GTK notification ("long task finished", "your model finished
      loading"). Works even when the window is minimized.

### Tier 3 — native-only superpowers / later

11. **Model lifecycle** (`list_models`, `load_model`, `unload_model`,
    `get_slot_stats`)
    - Direct llama-server control through the C bridge: switch models, watch
      slots/KV, manage a second slot for background tasks.
12. **Sandboxed shell** (`run_command`)
    - ⚠️ llama.cpp already has `exec_shell_command`, but it is **un-sandboxed**
      (runs as the server user with full privileges). If we use it, the app
      must add: explicit consent per call, hard timeout, and (ideally) a
      dedicated low-privilege user/workspace for the server itself. A fully
      sandboxed app-side alternative (no-network, cwd=workspace) remains an
      option for later.
13. **Local text extraction** (`extract_text`)
    - PDF (pdf.js already bundled), text files, EPUB; optional OCR hook
      (Tesseract) when present on the system.
14. **Memory / facts store** (`remember`, `recall`)
    - Opt-in local key-value store (JSON file in user data dir) so the agent
      can persist user preferences/facts across conversations.
15. **Timer / delayed action** (`schedule`)
    - Set a one-shot timer; fires a system message into the conversation
      (needs a background context — pair with a persistent slot).
16. **Audio transcription** (`transcribe`)
    - If a whisper-compatible endpoint/server is configured, transcribe an
      audio attachment; otherwise report unsupported.
17. **Keyboard input injection** (`type_text`) — *controversial; for the
    automation power-user setting only, likely out of scope.*
18. **Currency/unit conversion** (`convert_units`)
    - Static conversion table built in (offline); live rates only via the
      web-search provider if enabled.

## Cross-cutting notes

- **Schema source of truth**: tools are JSON schemas injected into the
  `/v1/chat/completions` `tools` array; implementations are services next to
  `sandbox.service.ts`.
- **Gating**: each tool gets a settings toggle (like `JavaScript sandbox tool`)
  and a permission policy (`never` / `ask` / `allow`).
- **Rendering**: extend the existing `parsers/` + tool-call cards; new
  renderers for table preview, diagram preview, notification status.
- **Server integration**: tools that need a background context (timer, model
  lifecycle) must coordinate with the slot/proxy layer (`server.c` bridge).

## Suggested order of implementation

0. **Enable llama.cpp built-ins**: forward `--tools` (or `--agent`) when the
   app later manages the server; verify `/tools` flows through the existing
   tools UI + parsers. Decide the workspace trust boundary.
1. `calculate` — trivial, immediate value
2. `fetch_url` via `/cors-proxy` (foundation for search)
3. `to_table`, `generate_mermaid`, `todo_list`
4. Clipboard + notifications (native bridge)
5. Web search (opt-in), model lifecycle (app-native — not in llama.cpp)
6. Sandboxed shell policy (consent/timeout/privileges), memory store, timers,
   transcription
