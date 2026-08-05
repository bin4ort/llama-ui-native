# REFACTOR-PLAN — Vanilla frontend rewrite (HTML/JS/CSS, no TypeScript)

Goal: replace the entire SvelteKit 5 + TypeScript frontend with plain
HTML/JavaScript/CSS — **zero** feature or visual loss. Native side stays C
(main.c/server.c already plain C; no C++ needed unless a future native module
demands it). Build tooling becomes minimal (esbuild + Tailwind CLI). The plan
is structured so **two agents can work in parallel without touching each
other's files**.

Status: IN PROGRESS — Phase 0 done (baseline + data snapshot, committed
f7bf252; plus a seeded-content baseline with verification harness, committed
cd845db). Phase 1 kernel foundation built, bootable and committed (c710ac7).
Phase 2 both verticals landed first cut: Agent A chat vertical (65c9104 —
streaming, composer, sidebar, search) and Agent B settings vertical
(6f8ca0a — all 8 settings sections, MCP page, preset manager + wizard,
log-level slider + debug log in Developer, import/export). Settings pages
verified headless with zero console errors; wizard generates drafts via the
live backend; logLevel persists in LlamaUi.config. Agent A's in-progress
chat edits remain uncommitted in the working tree (their files, untouched).

Kernel contract changes so far (logged per §4):
- `kernel/api.js` added: models/props/slots/health/completions + SSE stream
  reader (was planned in §2 but missing; codes taken from the frozen registry)
- `kernel/error-codes.js`: append-only additions CFG-005..010, MCP-008,
  PRS-005, TL-007
- `kernel/presets-store.js`: BUILTIN_PRESETS restored to the approved set
  (Psychologist, Brainstorming Partner, Productivity Coach, Socratic Thinking
  Partner, Creative Writing Editor) — the Phase 1 list contained programming
  presets ("Expert Programmer", "Strict Code Reviewer") which the product
  requirements explicitly rule out. Added `openPicker()`/`registerPicker()`
  (preset picker contract, plan §4) and `getByName()`.
- `kernel/modal.js`: added `mountModalHost()` — the modal primitive had no
  renderer, so dialogs built on showModal never displayed.
- `kernel/permissions.js`: `verify()`/`registerVerifier()` stubs existed;
  Agent B now registers the real VerificationDialog at boot
  (`web/settings/verify-dialog.js`).
- `web/index.js`: /settings/*, /settings/presets, /mcp-servers routes
  delegate to the settings module; modal host mounted; verifier + picker
  registered at boot; SW registered (skipped under ?native); e2e hook
  `?test=1` exposes `window.__kernel`.
- `web/styles/input.css`: imports settings.css (Agent B owned)
- Agent A additions (162eede): `kernel/ui.js` button/checkbox/collapsible
  primitives; build.mjs copies katex dist; mermaid/katex lazy blocks.

---

## 1. Why & what "bloat" means here

Current frontend measured (2026-08-04):

- `src/`: 292 `.svelte` files, 283 `.ts` files, ~110 000 LOC
- Runtime JS bundle: **~9.4 MB** single file (Svelte runtime + bits-ui +
  remark/mermaid/katex/pdfjs/etc.)
- Dev toolchain: SvelteKit, vite, storybook, vitest (+ browser project),
  svelte-check, dozens of TS-only deps

Target state:

- SPA in plain ES modules + HTML + CSS. No framework, no TS, no Svelte.
- Runtime bundle: expect ~1–2 MB before lazy chunks (mermaid/katex/pdfjs
  loaded on demand, like today's assets but chunked properly).
- Build: `esbuild` (bundle/minify) + `tailwindcss` CLI + small `build.mjs`.
- Same HTTP server, same native window, same data.

**Hard rules (no compromises):**

1. Every feature that exists today must exist after the swap — see the parity
   checklist (§6). Nothing is dropped "because it takes longer".
2. Visual parity: same layout, spacing, colors, themes, fonts, breakpoints.
   Verified with screenshot diffs, not eyeballing.
3. **Data compatibility:** localStorage keys (`LlamaUi.config`,
   `systemPromptPresetsSeeded`, `lang`, `mode-watcher-*`, …) and the
   **IndexedDB schema** (conversations/messages tables, indexes) stay
   byte-identical. Existing users lose nothing; `migration.service.ts`
   legacy handling is ported.
4. i18n: all 12 languages, same keys, same fallback behavior.
5. Native C server: unchanged except `FRONTEND_DIR` switching to the new
   output directory at swap time.

---

## 2. Target architecture

```
web/                     <- new frontend (parallel tree, strangler pattern)
  kernel/                <- shared foundation (frozen after Phase 1)
    router.js            hash router (#/chat/{id}, #/settings/{section}, #/search, #/mcp-servers)
    store.js             tiny reactive store (pub/sub + $state-style snapshot)
    api.js               HTTP client (completions SSE, models, props, slots, tools, MCP)
    i18n.js              t()/tr + embedded dicts (port of i18n.svelte.ts)
    theme.js             dark/light/system + custom theme tokens
    settings-store.js    LlamaUi.config persistence + server-prop sync
    db.js                Dexie wrapper (same schema as today)
    presets-store.js     preset library + one-time built-in seeding (marker key kept)
    permissions.js       staged always-allow + verification flow
    error-codes.js       stable LLMUI error-code registry (single source of truth)
    logger.js            leveled logger (console + ring buffer) — see §2.1
    modal.js, toast.js   shared primitives
    tokens.css           CSS custom properties per theme (port of current classes)
  app/                   <- AGENT A ownership
    shell/               window chrome, sidebar, conversation list, nav
    chat/                message tree, streaming, markdown pipeline, code/mermaid/katex,
                         attachments, audio/video/pdf, context gauge, search page
    chatbar/             composer, attach menu, persona quick picker, system prompt edit
  settings/              <- AGENT B ownership
    pages/               General/Display/Sampling/Penalties/Tools/Agentic/Developer/
                         Import-Export + MCP servers page
    presets/             preset manager, DialogPresetPicker, DialogPresetWizard (JS API: openPresetPicker())
    models/              model list/selection, slots, load states
    dialogs/             shared dialogs incl. VerificationDialog
    pwa/                 manifest + hand-written service worker
  styles/
    chat.css             <- Agent A
    settings.css         <- Agent B
    base.css             <- Phase 1 (resets, tokens, typography, fonts)
  index.html             <- Phase 1 (single shell; routes render via kernel router)
  assets/                favicons, icons, fonts (copied, not rebuilt)
  build.mjs              <- Phase 1 (esbuild + tailwind + copy + version stamp)
```

Output: `dist-web/` → copied to `frontend/v3/` at swap time. `frontend/v2/`
stays untouched until parity is proven; the C server switches
`FRONTEND_DIR` to `frontend/v3` only in Phase 3.

---

## 2.1 Error codes & leveled logging (new capability)

Every unexpected behavior in the app carries a **stable error code**, and a
**log-level slider** in Developer settings controls how much gets logged —
from "errors only" up to "everything without exception". This makes bug
reports self-describing ("I get LLMUI-STR-113") and reproductions cheap.

### 2.1.1 Code format

```
LLMUI-<AREA>-<NNN>
 ^    ^      ^-- 3 digits, 000–999, unique per area, append-only (never reused)
 |    +-------- 3-letter area (see table)
 +------------- project prefix
```

Examples: `LLMUI-API-012` (fetch models failed), `LLMUI-STR-113` (SSE stream
aborted mid-chunk), `LLMUI-SRV-201` (native server file not found).

Area registry (frontend):

| Area | Meaning |
|---|---|
| `SYS` | app boot / lifecycle / version |
| `CFG` | settings store, prop sync, reset |
| `DB` | IndexedDB (Dexie) / conversations / migrations |
| `API` | HTTP client (models, props, slots, tools, MCP transport) |
| `STR` | streaming / SSE / chat completion transport |
| `CHAT` | chat store, message tree, branching, titles |
| `MD` | markdown pipeline (marked, LaTeX, mermaid, svg blocks) |
| `IMG` | image attachments (resize, EXIF, HEIC) |
| `AUD` | audio (recording, files, transcription) |
| `PDF` | PDF → text extraction |
| `MCP` | MCP client, resources, prompts, sessions |
| `TL` | tools / agentic loop / permissions / verification |
| `PRS` | presets (library, seeding, wizard, change_preset tool) |
| `SEC` | sandbox, sanitization, redaction |
| `PWA` | service worker, manifest, updates |
| `UI` | generic UI failures (modal, toast, render) |
| `SRV` | **native C server** (own registry in `error-codes.h`) |

Rules:
- One code per failure site; the **registry is the single source of truth**
  (`web/kernel/error-codes.js` on the JS side, `error-codes.h` in C, kept in
  sync manually — both are tiny tables).
- Codes are **append-only and never reused**; a removed failure site leaves
  its code reserved with status "retired".
- The registry generates `docs/ERROR-CODES.md` during build (Phase 4 adds the
  generator; before that the doc is written by hand and kept in sync).
- UI surfaces that can fail show the code to the user:
  `"Error LLMUI-API-012 — could not reach the model server"` — and every
  issue filed against the app should cite its code (see ISSUES.md).

### 2.1.2 Log levels (slider in Developer settings)

| Level | Slider label | Content |
|---|---|---|
| 0 | Errors | critical failures only (limited, most important) |
| 1 | Warnings | + unexpected-but-recoverable conditions |
| 2 | Info (default) | + key lifecycle events (boot, model load, connect, settings applied) |
| 3 | Debug | + detailed flow (stream chunks, store mutations, permission decisions) |
| 4 | Trace | **everything without exception** (raw SSE frames, full payloads, every API call, DOM timings) |

- Setting key: `logLevel` (number 0–4), stored in `LlamaUi.config` like all
  other settings → rides settings export/import and can be shared with a bug
  report. Default: `2` (Info). Not synced to the server (client-only).
- Threshold semantics: severity ≤ selected level is logged
  (0=error … 4=trace).
- Sink: browser `console` (respecting the level: `console.error/warn/info/
  debug`) **plus** an in-memory ring buffer (last 500 entries) that the
  Developer settings page can show and copy ("Copy debug log" → plain-text
  lines `ts | level | LLMUI-AREA-NNN | message | detail`). The buffer is not
  persisted; it exists to make "send us the log" possible after a session.
- Logger API (kernel): `log.error(code, msg, detail?, err?)`,
  `log.warn/info/debug/trace(code, msg, detail?)` — the level filter applies
  inside the logger, so call sites never branch on level themselves.
- Error objects: `new LlmUiError('LLMUI-…', message, detail)` — thrown errors
  carry the code; the UI renders it from `error.code`.
- The native C server gets the same idea, scaled down: stderr lines prefixed
  `[LLMUI-SRV-NNN]`, level filter via env var `LLMUI_LOG_LEVEL` (0=errors,
  1=+warnings, 2=+info, default 1). No slider on the C side — it is small and
  its logs are developer-facing only.

### 2.1.3 Implementation split (no interference)

- **Phase 1 (kernel owner):** `error-codes.js` (frontend registry, initial
  ~60 codes covering every service), `logger.js` (levels, console + ring
  buffer, `LlmUiError`), `error-codes.h` (C side, ~15 codes), slider state in
  `settings-store.js` (`logLevel`, default 2).
- **Agent B** (Developer settings page): the log-level slider UI + "Copy
  debug log" button + a collapsible live log view (reads the kernel ring
  buffer only).
- **Agent A** (chat/services) and **Agent B** (settings/services): both
  instrument their own code with registry codes; they may only *use* codes,
  never edit the registry (kernel-frozen; new codes are contract changes).

---

## 3. Dependencies — keep / drop

Kept (all plain-JS, runtime): | Reason
---|---
`mermaid` | diagrams (lazy-loaded chunk)
`highlight.js` | code highlighting (already plain JS)
`katex` | math (lazy chunk, replaces rehype-katex)
`pdfjs-dist` | PDF → text (lazy chunk)
`dexie` | IndexedDB wrapper — schema compatibility is critical
`dompurify` | sanitization (SVG + HTML)
`fflate` | zip for import/export
`@modelcontextprotocol/sdk` | MCP client
`lucide` (vanilla pkg) | icons (replaces `lucide-svelte`; static SVG injection)

Replaced/dropped: | Replace with
---|---
Svelte / SvelteKit / svelte-check | plain ES modules + kernel router
`lucide-svelte` | `lucide` vanilla (`createIcons`)
bits-ui (shadcn svelte) | hand-rolled primitives in kernel (dialog, dropdown, checkbox, collapsible, tabs, tooltip, textarea auto-resize) — port the behavior, keep the classes
remark / rehype / unified / mdast / mdsvex | `marked` (+ GFM) + DOMPurify + ported latex-protection/literal-html/mermaid-blocks/svg-blocks/table-restorer plugins
`rehype-katex` | `katex.render` in the markdown post-processor
`@sveltejs/adapter-static`, vite PWA plugins | plain output + hand-written service worker (keep workbox only if parity proves hard — decision at Phase 2)
storybook, vitest, svelte-testing | playwright only (e2e, framework-agnostic)
`tailwindcss` v4 (CLI, build-time only, zero runtime) | KEEP — utility classes port 1:1, visual parity becomes tractable. Pure hand-written CSS is the fallback if Tailwind itself is considered bloat, but it multiplies the parity risk

JS-only note: all kept libs ship plain-JS/ESM builds — no TS needed in our
code; the few libs typed in TS are used via their JS entry points.

---

## 4. Two-agent work split (no file interference)

**Ownership rule:** after Phase 1, `web/kernel/**`, `web/index.html`,
`web/build.mjs`, `web/styles/base.css` are **frozen** — edits only via
reviewed contract changes. Agent A owns everything under `web/app/` +
`web/styles/chat.css`. Agent B owns everything under `web/settings/` +
`web/styles/settings.css`. No agent edits the other's tree. All cross-feature
needs are satisfied through kernel API; if a kernel change is required, it's
a contract change logged in the plan's changelog and applied by the agent
who owns the kernel (Phase 1 agent) — or, if neither owns it, jointly with
review.

Cross-feature surfaces (prevent interference by contract):

- Persona quick-picker (A, in chat bar) calls `kernel.presets.openPicker()`
  — the picker/wizard UI is implemented by **B** under `web/settings/presets/`
  and registered into the kernel API in Phase 1 as a stub.
- Chat attach menu (A) reads MCP resources/prompts through `kernel.mcp`
  getters — B owns the MCP servers page and MCP store internals.
- Context gauge (A) reads `kernel.chat` stream state.
- VerificationDialog (B) is invoked by kernel `permissions.verify()` — A's
  tool-call UI only calls the kernel function.
- Search page (A) uses `kernel.db` + `kernel.chat` only.
- PWA/service worker (B) must not cache anything Agent A hasn't finished —
  SW asset list is regenerated from a manifest file owned by B but written
  by `build.mjs` (shared, frozen).

File-level, per-agent work sets:

| Agent A — `web/app/**` | Agent B — `web/settings/**` |
|---|---|
| shell/layout/sidebar/conversation list | settings shell + all 8 sections |
| chat page: message tree, streaming SSE client, markdown pipeline (marked + plugins), code blocks, mermaid/katex blocks, reasoning rendering, tool-call rows | MCP servers page + MCP store |
| attachments: image/audio/video/text/PDF upload, preview, HEIC/JPEG-orientation, mic recording | preset manager + picker + wizard (UI + generation call) |
| composer, attach menu, persona quick-picker (thin shell around B's dialog), system-prompt edit | model management UI (list/load/slots/favorites) + models store |
| context gauge, auto-scroll, branching, search page | import/export UI (uses kernel db + fflate) |
| built-in tool **rendering** + permission prompts (calls kernel.verify) | built-in tools settings UI (staged always-allow, search) |
| error/empty/loading states in chat | verification dialog, toasts, all settings dialogs |
| instrument own code with LLMUI codes (use only, never edit registry) | PWA: manifest + service worker; **log-level slider + debug-log view in Developer settings** |

Shared work products both must NOT create: no new stores outside kernel, no
CSS outside own stylesheet, no global DOM ownership outside own subtree
(shell is A's, but B's pages render inside `<main id="page-root">` slot that
A owns — B only replaces that node's content via router contract).

---

## Plan changelog (contract changes)

- 2026-08-05 — `web/kernel/api.js` (Agent B authored, committed 6f8ca0a):
  accepted as the kernel HTTP client (baseUrl from serverEndpoint, request()
  with code maps, chatCompletion one-shot, generator-based streamChatCompletion).
- 2026-08-05 — **Contract change (Agent A, kernel owner):** the SSE generator
  now additionally yields `reasoning` and `toolCalls` fields alongside
  `delta`/`finish` (additive — existing consumers unaffected). Needed by the
  agentic loop; no renumbering of codes.

## 5. Phases

### Phase 0 — Freeze scope (0.5–1 day, parallel-safe)
- Record current behavior as screenshots (playwright script, existing UI on
  port 8765, ~25 canonical views incl. all themes, all settings sections,
  picker, wizard, search, MCP page).
- Snapshot IndexedDB + localStorage key names from code.
- Lock the parity checklist (§6) with checkboxes.
- Cut-off note: bin4ort's in-flight features (presets restyle, built-in
  tools, persona stacking) are **in scope** — they keep working in `src/`
  while we port; their final state at the swap date is the parity target.

### Phase 0 — Data keys snapshot (taken 2026-08-04)

IndexedDB (Dexie):
- DB name: `LlamaUi`, version 1
- `conversations` — indexes: `id, lastModified, currNode, name`
- `messages` — indexes: `id, convId, type, role, timestamp, parent, children`

localStorage (prefix `LlamaUi.`):
- `LlamaUi.config` (settings JSON incl. `systemPromptPresets`, `logLevel` later)
- `LlamaUi.alwaysAllowedTools` · `LlamaUi.disabledTools` · `LlamaUi.disabledToolKeys`
- `LlamaUi.favoriteModels` · `LlamaUi.reasoningEffortDefault` · `LlamaUi.userOverrides`
- `LlamaUi.dismissedRecommendedMcpServers` · `LlamaUi.buildVersion` · `LlamaUi.todos`
- `LlamaUi.streamResume.<convId>` (per-conversation prefix)
- bare keys: `lang`, `systemPromptPresetsSeeded` (one-time built-in presets marker)
- legacy (`LlamaCppWebui.*`): config, alwaysAllowedTools, disabledTools, favoriteModels, userOverrides — port migration

Phase 0 screenshot harness: `scripts/parity-screenshots.mjs` (playwright), output to `parity-baseline/`.

### Phase 1 — Foundation (sequential, ONE agent; ~3–5 days)
Build `web/` skeleton: `index.html`, `build.mjs` (esbuild + tailwind +
copy + `--watch` dev mode), `kernel/*` (router, store, api, i18n with all 12
embedded dicts, theme tokens incl. all 11 themes, settings-store with prop
sync incl. `logLevel`, db with exact schema, presets-store incl. seeding
marker logic, permissions/verification, **error-codes.js registry + logger.js
(levels, ring buffer, LlmUiError)**, modal/toast), `base.css`, fonts/assets
copy, playwright harness + first screenshot baseline.
Output: a bootable shell that renders the chat empty-state and settings
placeholder using the real stores. Kernel freeze + contract doc written.

### Phase 2 — Parallel verticals (both agents; ~2–3 weeks)
Agent A and Agent B implement their trees per §4 against the frozen kernel.
Each keeps the app bootable: A owns the shell, so A's work is always
visible; B's pages mount into the page-root slot via the router. B tests via
`#/settings/...` routes; A via `#/chat/...`. Both run
`npm run build:web` + screenshot harness on their own branch... (no —
single branch, disjoint files, so no conflict: commit as you go, rebase is
trivial since no overlapping paths).
Both agents instrument their code with LLMUI codes (use-only; registry
extensions are contract changes). B ships the log-level slider + debug-log
view in Developer settings.

### Phase 3 — Parity gate (joint, sequential)
- Full screenshot diff vs Phase 0 baseline (threshold: pixel-diff < 1% per
  view, or documented intentional change).
- Functional walkthrough of every checklist item.
- E2E: port the meaningful playwright specs (streaming, send message, preset
  apply, import/export round-trip, MCP connect, tool permission flow,
  i18n switch) + new: log-level slider honors threshold; trace captures
  everything; ring buffer copy produces parseable lines.
- Fix ISSUES.md #1–#12 in the new code (do NOT port bugs).
- Swap: `server.h` FRONTEND_DIR → `frontend/v3`; rebuild C; run the real
  app; verify `/health`, static, SSE proxy, seeding on fresh profile, and
  `LLMUI-SRV-*` codes appear in stderr on forced server errors.

### Phase 4 — Decommission & docs (0.5–1 day)
- Delete `src/`, svelte configs, vite.config, storybook, vitest, svelte-check
  scripts; prune package.json devDeps; keep only esbuild/tailwind/playwright.
- Rewrite AGENTS.md (build workflow), README (structure, build), CHANGELOG
  (0.5.0 major), bump `APP_VERSION`/`server.h`/`package.json`.
- Add `docs/ERROR-CODES.md` generator (build step emitting the registry table
  from `error-codes.js` + `error-codes.h`).
- Remove `frontend/v2` after a grace period; keep `lang/*.json` + `i18n.js`
  semantics inside the new build (embedded dicts remain the source).

---

## 6. Feature parity checklist (locked target)

Chat core
- [ ] conversation tree (root/persona/system/user/assistant branches, branching/re-edit)
- [ ] streaming SSE (`/v1/chat/completions` via our server), abort/regenerate/continue, pending messages during stream
- [ ] title generation (first line + LLM), rename, delete, export/import conversations
- [ ] markdown: GFM tables/lists/links, LaTeX (inline+display) with latex-protection, literal-html handling, code blocks with hljs + copy, mermaid, inline SVG blocks with sanitize, image pasting
- [ ] reasoning blocks (`<think>` + `<<<reasoning_content_start>>>` legacy), tool-call rendering
- [ ] attachments: image (resize, EXIF-orientation, HEIC), audio (mic + file), video, text, PDF (pdfjs→text); modality gating per model
- [ ] context gauge + auto-scroll + "and N more" + editing messages
- [ ] search page (full-text over conversations/messages)

Chat bar
- [ ] composer: auto-resize, IME guard, shortcuts, draft persistence
- [ ] attach menu (all types + disabled tooltips), MCP prompts/resources entries
- [ ] persona quick-picker (Default + favorites + "All presets…"), system prompt edit
- [ ] pending-message queue during streaming

Settings (all 8 sections + MCP servers page)
- [ ] General (language incl. 12 langs, theme incl. 11 themes, model, …)
- [ ] Display / Sampling / Penalties — every slider/checkbox with server-prop sync + userOverrides
- [ ] Tools: per-tool toggles, built-in tools group, staged always-allow, verification dialog
- [ ] Agentic: max turns, tool-call formatting, list/change preset tools toggle
- [ ] Developer: API key, server endpoint, custom JSON, custom CSS, JS sandbox flags
- [ ] Developer: **log-level slider (0 Errors … 4 Trace, default Info)** + "Copy debug log" + collapsible live log view (kernel ring buffer)
- [ ] Import-Export: settings + conversations, zip round-trip (fflate)
- [ ] MCP servers: add/edit/delete, connect test, resources/prompts, recommended servers list
- [ ] Prompt presets: manager (search/star/edit/delete), picker dialog, wizard (generation + JSON parse), one-time built-in seeding of the 5 presets
- [ ] models: list, load/unload states, favorites, slots view
- [ ] reset settings, localStorage note, about/version footer (APP_VERSION/APP_BUILD)

Cross-cutting
- [ ] agentic loop: built-in tools (fetch_url incl. CORS proxy path, calculate, weather, todos, JSON/table tools, file tools via native side), tool permission prompts, injection of steering/pending messages
- [ ] PWA: manifest, service worker (offline shell + hashed asset caching), splash screen, favicon set
- [ ] i18n: 12 languages, same keys, key-fallback, `lang/*.json` still source of truth
- [ ] data: identical localStorage keys + IndexedDB schema; legacy migration ported
- [ ] themes: system/dark/light + amoled/nord/dracula/cobalt/solarized/gruvbox/ionized-purple/ionized-red/ionized-cyan/snow/gruvbox-light
- [ ] native flags: `?native=1`, `--tools`/`--api-key`/`--ui-mcp-proxy` CLI flags, server health/version display

Logging & error codes (new, §2.1)
- [ ] `LLMUI-<AREA>-<NNN>` codes on every failure site (frontend registry + C `error-codes.h`); codes never reused
- [ ] `logLevel` slider (0–4, default 2) in Developer settings, persisted in `LlamaUi.config`, included in export/import
- [ ] logger honors threshold; level 4 (Trace) logs everything without exception (raw SSE frames, full payloads)
- [ ] in-memory ring buffer (500) + "Copy debug log" (parseable `ts | level | code | message` lines)
- [ ] `LlmUiError` carries code; user-visible errors show "Error LLMUI-…"
- [ ] C server stderr uses `[LLMUI-SRV-NNN]` + `LLMUI_LOG_LEVEL` env filter (0/1/2)
- [ ] `docs/ERROR-CODES.md` generated from the registries at build time

---

## 7. Risks & mitigations

| Risk | Mitigation |
|---|---|
| Visual drift | Tailwind kept + screenshot baseline in Phase 0, pixel-diff gate in Phase 3 |
| Markdown behavior differences | Port the exact plugin chain semantics (latex-protection, blocks, restorer) with unit fixtures extracted from current output; test with the same documents |
| Data loss on swap | Identical keys/schema; migration port; import/export round-trip e2e before swap; `frontend/v2` kept until Phase 4 |
| Streaming subtleties (resume, SSE error events, abort) | Port `chat.service.ts` logic 1:1; e2e against live llama.cpp server |
| Lazy chunks breaking SPA | esbuild splitting + `import()`; SW must not pre-cache lazy chunks (runtime cache only) |
| Two-agent interference | Frozen kernel + disjoint trees + no shared files outside own subtree; contract-change protocol in §4 |
| bin4ort concurrent commits | Refactor never touches `src/`; swap date pins their feature set; ISSUES.md items are port-time requirements, not blockers for them |
| 9.4 MB bundle regression | Size budget: initial < 2.5 MB gz, lazy chunks per feature; measured in CI-style check during Phase 2 |

---

## 8. Deliverables & exit criteria

- `web/` tree fully implementing §6 with zero unchecked boxes.
- Screenshot diff ≤ 1% per baseline view; e2e green; ISSUES.md #1–#12 closed
  (as fixed-in-rewrite, verified).
- `frontend/v3` served by the C binary; fresh-profile seed test passes.
- `src/` deleted; build = `npm run build` → esbuild+tailwind; docs updated;
  version bumped to 0.5.0.
- Both agents' work committed as disjoint file sets (git history shows zero
  cross-ownership edits).
