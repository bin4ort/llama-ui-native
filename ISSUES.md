# Issue List

Tracking file for known bugs, errors and code-quality problems. Filled in by
the reviewing agent (stry4ok side); the other agent (bin4ort) works on the
preset/tools features — re-check this list when their commits land.

Status legend: `OPEN` / `FIXED` / `VERIFY` (needs confirmation) / `WONTDO`

---

## Error codes & logging (design — target of the rewrite, see REFACTOR-PLAN.md §2.1)

Every failure site will carry a stable code **`LLMUI-<AREA>-<NNN>`**
(area table in the plan; frontend registry `web/kernel/error-codes.js`,
native registry `error-codes.h`). Codes are append-only, never reused, and
user-visible errors show the code so bugs can be reported verbatim.

A **log-level slider** (Developer settings, key `logLevel`, default 2) sets
the threshold:

| Level | Label | Logged content |
|---|---|---|
| 0 | Errors | critical failures only (limited, most important) |
| 1 | Warnings | + unexpected-but-recoverable conditions |
| 2 | Info | + key lifecycle events (default) |
| 3 | Debug | + detailed flow |
| 4 | Trace | everything without exception |

Sinks: console + in-memory ring buffer (500 entries) with a "Copy debug log"
button in Developer settings. Native C server: `[LLMUI-SRV-NNN]` stderr
lines, `LLMUI_LOG_LEVEL` env filter (0/1/2).

Conventions for this file:
- New findings should include the area (+ proposed code, e.g. `PRS-2xx`
  range reserved for presets) so the registry can assign it during the
  rewrite.
- Issues #1–#12 below are **port-time requirements**: they must be FIXED in
  the new implementation, not carried over.

Implementation tracking (rewrite scope):
- [ ] `error-codes.js` registry (frontend) + `error-codes.h` (C) — Phase 1 kernel
- [ ] `logger.js` — levels, console + ring buffer (500), `LlmUiError` — Phase 1 kernel
- [ ] `logLevel` setting (key `logLevel`, default 2) in `LlamaUi.config` — Phase 1 kernel
- [ ] slider UI + "Copy debug log" + live log view in Developer settings — Phase 2, Agent B
- [ ] LLMUI-code instrumentation of all services — Phase 2, both agents (use-only)
- [ ] C server `[LLMUI-SRV-NNN]` stderr + `LLMUI_LOG_LEVEL` env filter — Phase 1/2
- [ ] `docs/ERROR-CODES.md` generator — Phase 4

---

## Frontend — TypeScript errors (`npm run check` → svelte-check)

8 errors in 4 files, all in the preset-settings work (bin4ort WIP). These
block a clean `npm run check` and should be fixed before merge.

| # | File:line | Error |
|---|-----------|-------|
| 1 | `src/lib/components/app/settings/SettingsChat/SettingsChat.svelte:53,80` | `Variable 'mobileHeader' implicitly has type 'any'` (2×) |
| 2 | `SettingsChat.svelte:69` | `Parameter 'newTheme' implicitly has an 'any' type` |
| 3 | `SettingsChat.svelte:74` | `Parameter 'key' / 'value' implicitly has an 'any' type` |
| 4 | `SettingsChatFields.svelte:154` | `Parameter 'opt' implicitly has an 'any' type` |
| 5 | `SettingsChatFields.svelte:241` | `Parameter 'o' implicitly has an 'any' type` |
| 6 | `src/routes/settings/[[section]]/+page.svelte:15` | `Property 'getSectionHref' is missing in type '{ initialSection: string | undefined; }' but required in type '$$ComponentProps'` — the page must pass `getSectionHref` (likely `RouterService.settings`) |

Fix suggestion: type the event params (`$bindable`/props on the mobile header,
`event: { currentTarget: HTMLSelectElement }` etc.) and pass a
`getSectionHref` prop from the route page.

---

## Preset stacking (feature from commit `f261b15`) — behavioral bugs

### #7 — Applying a preset silently fails on conversations that have a context system message but no persona row
- **Code (proposed):** `LLMUI-PRS-201`
- **Status:** OPEN (blocked on #1–#6 since the picker code isn't done yet — verify after those land)
- **File:** `src/lib/stores/chat.svelte.ts`
- **Cause:** `applySystemPromptContent()` (line ~930) only finds a row with
  `type === 'persona'`. When none exists it falls back to
  `addSystemPrompt()` (line ~973), whose early-return guard matches
  `m.role === MessageRole.SYSTEM && m.parent === rootId` — **both** persona and
  context rows have `role: SYSTEM` (`createSystemMessage` sets the role always,
  the 4th arg only sets `type`). So on a conversation with a pre-feature
  `'system'` row (all conversations created before this change) the preset
  application hits the guard, sets `pendingEditMessageId` (opens the context
  row editor) and returns **without applying the persona**.
- **Impact:** any pre-upgrade conversation (or one where the user edited the
  system message via "Edit system message…") cannot get a preset applied.
- **Repro:** create conversation with a system message (old type), open picker,
  select a preset → nothing changes.

### #8 — Removing a preset from an *inactive* conversation leaves a ghost persona row
- **Code (proposed):** `LLMUI-PRS-202`
- **Status:** OPEN
- **File:** `chat.svelte.ts:948-955`
- **Cause:** the empty-target branch only calls
  `removeSystemPromptPlaceholder()` for the **active** conversation; otherwise
  it writes `content: ''` into the DB (`updateMessage`). The ghost row is
  exactly what `d925cf6` fixed for the active path, but the inactive path still
  creates it.
- **Impact:** opening such a conversation later shows an empty persona bubble
  with only action icons.
- **Repro:** conversation A not open → switch the persona of A to Default via
  the picker/`change_preset` tool → open A → ghost bubble.

### #9 — First-message chain is anchored under the persona row
- **Code (proposed):** `LLMUI-PRS-203`
- **Status:** VERIFY
- **File:** `chat.svelte.ts:1071-1084`
- **Cause:** for a new conversation the user message is created with
  `parentIdForUserMessage = systemMessage.id` (the persona row becomes the
  parent of the whole thread).
- **Concern:** `removeSystemPromptPlaceholder()` reparents children to root, so
  deletion is handled — but verify ordering/anchoring is still correct when the
  persona row is replaced or when a *second* branch (re-edit/insert) is added,
  and that "persona first" ordering is preserved in the sent request for
  conversations that went through `applySystemPromptContent` mid-thread.

---

## Built-in tools (committed in `4c7dc5a` / `208bfd1`) — suspected

### #10 — `fetch_url` will hit CORS for most external sites
- **Code (proposed):** `LLMUI-TL-101`
- **Status:** VERIFY
- **File:** `src/lib/services/builtin-tools.service.ts:196-207`
- **Cause:** the tool runs in the browser (`fetch(url)` from origin
  `http://localhost:8765`). Almost no public site sends
  `Access-Control-Allow-Origin: *`, so `fetch` will be rejected before the
  response is read. The old app had a `/cors-proxy` route for this.
- **Impact:** the "fetch full page content" tool likely fails in practice
  unless routed through a proxy.
- **Note:** also check the `1 MB cap` order — the `text.length > MAX_FETCH_CHARS`
  check happens *after* `response.text()` (already downloaded); fine, just
  memory-bound.

---

## Native server (`server.c`, self-written replacement) — minor

### #11 — Data race on `g_listen_fd` during shutdown
- **Code (proposed):** `LLMUI-SRV-011`
- **Status:** OPEN (low)
- **File:** `server.c`
- **Cause:** `g_listen_fd` is written by the server thread and read+shutdown by
  the main thread in `server_stop()`; `volatile` is not a memory barrier.
- **Impact:** benign in practice (single shutdown), but should be a proper
  `pthread_mutex`/`_Atomic int`.

### #12 — POST bodies without `Content-Length` are dropped; chunked encoding unsupported
- **Code (proposed):** `LLMUI-SRV-012`
- **Status:** OPEN (low)
- **File:** `server.c` (`parse_request`, body gather loop)
- **Impact:** the SPA always sends `Content-Length` via `fetch`, so nothing
  breaks today. Note it for future clients (e.g. curl `-T -`, non-JS tools).

---

## Fixed already (keep for reference)

- **FIXED (bin4ort, `d925cf6`)** — switching to Default preset left a ghost
  system bubble (active conversation path). Reparenting + removal added.
- **FIXED (bin4ort, `f652a84` / `4861c71`)** — preset active-check vs delete
  button ordering.
- **FIXED (stry4ok)** — AGENTS.md falsely claimed dist→frontend/v2 is copied
  automatically; the manual copy command is now documented (AGENTS.md, README).
- **FIXED (stry4ok)** — wizard `max_tokens: 500` too small for detailed
  system prompts → 1200; temperature 0.6 → 0.4 for JSON reliability.
- **FIXED (stry4ok)** — built-in preset seeding didn't fire until a
  conversation existed and the `'[]'` default slipped past the empty-check →
  eager module-load seed + `systemPromptPresetsSeeded` marker.
- **FIXED (stry4ok)** — license incompatibility: removed vendored Mongoose
  (GPL-2.0-only, incompatible with GPL-3.0-or-later) → self-written HTTP server.
  NOTE for bin4ort: do **not** restore `mongoose.c/h` — intentional.

---

## Environment / by-design

- The binary requires `LD_LIBRARY_PATH=.` (or `launch.sh`) for the bundled
  `libjxl.so.0.12` symlinks — by design.
- `WebKitGTK` console: `Viewport argument key "interactive-widget" not
  recognized and ignored` — harmless WebKit warning, not a real issue.
- Running the app twice: the old instance keeps port 8765 and the new binary
  silently fails to bind (server_start returns -1, window shows a dead page).
  Consider a "port already in use" dialog.

## Review log (bin4ort commits checked)

| Commit | Checked | Result |
|--------|---------|--------|
| `df31e68` restore mongoose | 2026-08-04 | Misunderstanding — removal is intentional (license); re-removed |
| `f261b15` preset stacking | 2026-08-04 | Found #7, #8, #9 |
| `d925cf6` ghost bubble fix | 2026-08-04 | Fixes active path only — #8 (inactive path) remains |
| `4c7dc5a` / `208bfd1` built-in tools | 2026-08-04 | Found #10 (CORS), committed |
| `ec5f153` presets restyle | 2026-08-04 | UI only; #1–#6 unchanged (8 svelte-check errors remain) |
| `664a2cf` tooltips/preset edit resizable | 2026-08-04 | UI polish; no new findings |
| `6af3243` tool-count key + preset name cap | 2026-08-04 | i18n fix; no new findings |
| `43d0e58` release v0.4.3 | 2026-08-04 | Version bump swept in my uncommitted server.h/package.json edits (kept); binary rebuilt → 0.4.3 / 0x07D21 |

