# Draft — Day-to-Day Custom Tools (not in llama.cpp)

> Status: **PROPOSED — not implemented.** A curated list of useful everyday
> tools to build into Llama UI Native. Deliberately excludes what llama.cpp
> already provides (`--tools`: read/write/edit_file, file_glob_search,
> grep_search, exec_shell_command, get_datetime, get_info) and what MCP
> servers can cover. Nothing here is committed yet.
>
> See also: `docs/builtin-tools-draft.md` (broader, tiered roadmap).

## Selection principles

- **Everyday value**: each tool should be something a normal user reaches for
  several times a week ("what's the weather", "summarize this URL", "copy
  that", "turn this CSV into a table").
- **No API keys by default**: prefer free, keyless services (Open-Meteo,
  Wikipedia API) or offline computation. Anything requiring an account is
  opt-in.
- **Trusted tools are tiny**: deterministic helpers (arithmetic, JSON, tables,
  diffs) need no sandbox and no permissions — implement them first.
- **Consent where it touches the machine**: clipboard, notifications,
  screenshots, local memory → per-call Allow once / Deny.
- **Not re-implementing**: no file editing, no shell (llama.cpp), no
  date/time (llama.cpp), no MCP-dependent glue.

## Tier A — everyday basics (small, deterministic, high frequency)

1. **Calculator** (`calculate`)
   - Arbitrary-precision arithmetic + expression parser (Decimal/BigInt).
   - Models still fail basic arithmetic in prose; this is the single most
     used day-to-day tool. Zero permissions.
2. **Read a webpage** (`fetch_url`)
   - GET any URL through the app's local `/cors-proxy`; HTML→text/markdown
     extraction, 1 MB cap, timeout, content-type sniffing.
   - "Summarize this article", "check this docs page", "what's on that page".
     Foundation for everything web.
3. **Table from data** (`to_table`)
   - CSV / TSV / JSON → formatted table preview (and markdown block).
   - Everyday: budgets, logs, exports, rankings.
4. **JSON helper** (`json_tool`)
   - Validate, pretty-print, flatten/query (JSONPath-lite) JSON.
   - Daily for anyone pasting configs/API responses into chat.
5. **Clipboard** (`read_clipboard`, `write_clipboard`)
   - GTK clipboard read (needs explicit approval — privacy) and write
     ("copy this code block", "copy this result").
   - Native app advantage; no browser permission prompts.
6. **Desktop notification** (`notify`)
   - GTK notification ("your 1-hour export is done", "long task finished").
   - Pairs with streaming tasks; works with the window minimized.
7. **Task list** (`todo_list`)
   - Persistent per-conversation todo (add/complete/list/clear), stored with
     the conversation.
   - Keeps multi-step agentic work honest across turns.

## Tier B — information lookup

8. **Weather** (`weather`)
   - Current conditions + short forecast via Open-Meteo (free, **no API key**,
     no account, privacy-friendly).
   - "Do I need a jacket tomorrow" — the classic everyday ask.
9. **Wikipedia lookup** (`wikipedia`)
   - Article summary/abstract via the Wikipedia REST API (free, keyless).
   - Quick factual grounding without a full web search.
10. **Web search** (`web_search`)
    - Opt-in: SearXNG instance / DuckDuckGo / user-provided API key. Off by
      default; nothing leaves the machine until enabled.
    - When enabled it becomes the daily driver for "current info".
11. **Unit & currency conversion** (`convert_units`)
    - Offline static unit table (length/mass/temp/data…); currency rates only
      via the search provider or a keyed feed when configured.

## Tier C — documents & content

12. **Text extraction** (`extract_text`)
    - PDF (pdf.js is already bundled), DOCX/ODT/EPUB → plain text/markdown.
    - "Summarize this PDF", "what does this report say".
13. **Diagram validation** (`mermaid`)
    - Validate a mermaid spec and render it to an SVG preview card in-chat
      (mermaid is already bundled for rendering).
    - Everyday for planning, flows, architecture sketches.
14. **Text diff** (`diff_text`)
    - Line/word diff between two texts/drafts (LCS or `diff`-style output).
    - "Compare my two versions", "what changed between these".
15. **Regex tester** (`regex_test`)
    - Test a pattern against sample text with match groups and an
      explanation of matches.
    - Tiny, deterministic, constant dev use.

## Tier D — personal & machine (consent-gated)

16. **Facts memory** (`remember`, `recall`)
    - Opt-in local key-value store (JSON file in the user data dir): "my
      brother's name is…", "I prefer dark mode and German".
    - Gives the assistant continuity across conversations.
17. **Reminder / timer** (`remind_me`)
    - One-shot delayed GTK notification ("remind me to take a break in
      20 minutes").
18. **Screenshot** (`capture_screen`)
    - Capture the current window/screen (native), attach as image — only
      useful with a vision model; explicit consent per call, redaction note.
    - "What's wrong with this UI?", "read this error on my screen".
19. **Transcription** (`transcribe`)
    - Transcribe audio attachments via a configured whisper-compatible
      endpoint; report unsupported otherwise.
    - Day-to-day for voice notes and meetings (local-first when possible).

## Explicit non-goals (for now)

- **Email / calendar / social / smart-home**: require accounts, APIs and
  trust decisions — leave to MCP servers and integrations.
- **Anything llama.cpp already ships** (file/shell/date/time/info) — surface
  those via `--tools` instead of re-implementing.
- **Generic SQL runner**: too powerful for the default trust model; `sqlite3`
  is reachable through the (cautious) shell policy if ever enabled.

## Suggested implementation order

1. Tier A 1–4 (`calculate`, `fetch_url`, `to_table`, `json_tool`) — no
   permissions, immediate everyday value
2. Tier A 5–7 (clipboard, notify, todo) — native bridge, consent-gated
3. Tier B (weather, wikipedia, search opt-in, convert_units)
4. Tier C (extract_text, mermaid, diff_text, regex_test)
5. Tier D (memory, reminders, screenshot, transcription)

## Cross-cutting

- Tools are JSON schemas appended to the `/v1/chat/completions` `tools` array
  (source: `frontend`), gated by settings toggles and the existing
  Allow once / Deny permission UI.
- Reuse the tool-call card renderers; add renderers for table/diagram/
  notification status.
- Memory and todo state live in the user data dir (JSON), never in
  localStorage only.
