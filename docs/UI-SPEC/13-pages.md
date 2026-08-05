# 13 — Pages & Routes

## 13.1 Root layout (+layout.svelte)

- Shell grid: sidebar strip (mobile drawer / desktop icon strip, §02) +
  main `page-root`; toasts host; modal host; theme classes on `<html>`
  (`.dark`, palette class, `data-theme`).
- PWA/app chrome: splash overlay at boot (logo + spinner, 400 ms min).

## 13.2 Chat page `(chat)/+page.svelte`

- Empty conversation state: renders ChatScreen with `showCenteredEmpty`
  (greeting + suggestion chips); creates a conversation on first visit.
- Query params: `?q=…` sends a message after load; `?model=…` preselects
  a model (opens ModelNotAvailable dialog if unknown).

## 13.3 Chat page `(chat)/chat/[id]/+page.svelte`

- Loads the conversation by id (`loadConversation`); sets document title
  `{conversationName} - llama-ui`; on unknown id redirects to `#/`.
- Model-sync: selects the model from the last assistant message after
  navigate; re-discovers active streams on visibility change.

## 13.4 Search page `search/+page.svelte`

- Layout: `mx-auto max-w-3xl p-6` — SearchInput (autofocus, `text-lg`
  input) + results header `flex items-center justify-between text-xs
  text-muted-foreground` ("N results") + result list.
- Result groups: "Conversations" / "Messages"; conversation rows like
  sidebar items; message rows `rounded-lg border border-border/30 p-3
  hover:bg-accent/50` — preview `text-sm line-clamp-2` with `<mark>
  bg-accent text-accent-foreground` highlights, meta `text-xs
  text-muted-foreground` (conversation name · date), click → open
  conversation.

## 13.5 Settings page `settings/[[section]]/+page.svelte`

- Wraps SettingsChat (§09); missing section in the URL → replaceState to
  `#/settings/general`.

## 13.6 MCP servers page `mcp-servers/+page.svelte`

- McpServers (§10.1) full page.

## 13.7 Error page `+error.svelte`

- Centered `flex flex-col items-center justify-center gap-3 p-8` —
  AlertTriangle 32 px `text-destructive`, `text-base font-semibold`
  ("Page not found" / error message), `text-sm text-muted-foreground`
  detail, "Back to chat" button (default).

## 13.8 Native window specifics

- Entry URL: `http://localhost:8765/?native=1` — `native` query disables
  the service worker; window chrome handled by GTK (title set from
  `document.title`).
- `?autoapprove=1` (e2e) auto-approves tool permissions;
  `?test=1` (e2e) exposes `window.__kernel`.
