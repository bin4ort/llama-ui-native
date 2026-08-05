# 10 — MCP (servers, resources)

## 10.1 McpServers page

- `mx-auto max-w-3xl p-6` — page title row (Database icon 20 px + `h1
  text-xl font-semibold`), description `text-sm text-muted-foreground`,
  "Add server" button (primary, Plus 16 px), server card grid
  `grid gap-4` (McpServerCard list), empty state
  `py-12 text-center text-sm text-muted-foreground` + hint.

## 10.2 McpServerCard

- `!gap-3 bg-muted/30 p-4` (grid, muted panel, 16 px padding) + border
  `border border-border/30 rounded-xl`.
- Header (McpServerCardHeader): `flex items-start justify-between gap-3` —
  logo `h-10 w-10 rounded-lg` + name `text-sm font-semibold` + URL
  `text-xs text-muted-foreground truncate`; connection state badge
  (connected green / connecting amber spinner / failed red);
  actions row (McpServerCardActions): edit / delete icon buttons
  `h-7 w-7` (hover:bg-accent; delete `text-destructive
  hover:bg-destructive/10`).
- Capability badges (McpCapabilitiesBadges): pills `h-5 gap-1 px-1.5
  text-[10px]` — tools `bg-green-50 text-green-700 dark:bg-green-950
  dark:text-green-400` (Wrench 12 px), resources `bg-blue-50 text-blue-700
  dark:bg-blue-950 dark:text-blue-400` (Database), prompts
  `bg-purple-50 text-purple-700 dark:bg-purple-950 dark:text-purple-400`
  (MessageSquare).
- Tools list (McpServerCardToolsList): toggle header
  `flex w-full items-center gap-1 text-xs text-muted-foreground
  hover:text-foreground` + chevron; items `max-h-64 space-y-3 overflow-y-auto`
  each `flex w-full items-center gap-1 text-xs` (tool name `font-mono
  text-xs` + description `text-muted-foreground`).
- Connection logs (McpConnectionLogs): collapsible `max-h-64 space-y-0.5
  overflow-y-auto rounded bg-muted/50 p-2 font-mono text-[11px]` — lines
  `flex w-full items-center gap-1 text-xs text-muted-foreground` with
  timestamps.
- Info (McpServerInfo): transport + enabled state
  `flex w-full items-center gap-1 text-xs text-muted-foreground`.
- Identity (McpServerIdentity): logo `flex min-w-0 items-center gap-1.5` +
  transport chip `h-4 max-w-24 min-w-0 shrink px-1 text-[10px]` + server
  name truncated.
- Skeleton (McpServerCardSkeleton): `grid gap-3 p-4` — Skeleton bars
  `h-5 w-5 rounded`, `h-5 w-28`, `h-5 w-12 rounded-full`.
- Compact (McpServerCardCompact, sidebar/composer): `flex min-w-0
  items-center gap-2` + `h-5 w-5 shrink-0 rounded` + name `truncate
  font-medium`.
- Edit form (McpServerCardEditForm): inline `space-y-4` (name, URL,
  transport select, headers KeyValuePairs, auth toggles, test) + footer
  `flex items-center justify-end gap-2`.
- Delete dialog (McpServerCardDeleteDialog): destructive action
  `bg-destructive text-destructive-foreground hover:bg-destructive/90`.

## 10.3 McpServerForm (add/edit)

- `grid gap-2`; labels `mb-2 block text-xs font-medium select-none`;
  errors `mt-1.5 text-xs text-destructive`; auth checkbox row
  `flex items-center gap-2 cursor-pointer select-none`.

## 10.4 Resources browser

- Browser header: `mb-2 flex items-center gap-4` — refresh button
  `h-8 w-8 p-0`, search `flex-1`, count `text-sm font-medium`; body
  `flex flex-col gap-1` with server groups; server item
  `flex w-full items-center gap-2 rounded px-2 py-1 text-sm hover:bg-muted/50`
  + chevron + resource rows `ml-4 flex flex-col gap-0.5 border-l
  border-border/50 pl-3` (icon 14 px + name `font-medium` + uri
  `text-xs text-muted-foreground font-mono`).
- Empty state: `py-4 text-center text-sm text-muted-foreground`.

## 10.5 Avatars & logo

- McpActiveServersAvatars: `flex -space-x-1` — each avatar
  `box-shadow-lg overflow-hidden rounded-full bg-muted ring-1 ring-muted`
  (24 px); "+n" chip `text-xs text-muted-foreground`.
- McpLogo: the MCP "M" mark — `frame-clip` SVG with layered fills
  (`frame-background`), 24–28 px, used in sidebar strip + server cards.
- McpResourcePreview: empty state `flex flex-col items-center justify-center
  gap-2 py-8 text-muted-foreground` (icon 32 px opacity-50) + resource
  header `flex items-start justify-between gap-2` (name `truncate
  font-medium` + type chip) + content area.
- McpResourceTemplateForm: `space-y-3` — argument rows
  `rounded-md bg-muted/50 px-3 py-2`, value preview `mt-0.5 font-mono
  text-xs break-all`, footer `flex justify-end gap-2`.
