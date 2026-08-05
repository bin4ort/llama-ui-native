# 02 — Shell & Navigation

## 2.1 SidebarNavigation

The app's primary chrome — a mobile drawer / desktop collapsed icon strip.
Full geometry in `../UI-SPEC.md` §3.17.

- **Structure**: `aside.fixed md:sticky top-2 left-2 md:left-0 md:ml-2
  md:mt-2 pt-2 z-10 w-[calc(100dvw-1rem)] md:h-[calc(100dvh-1.125rem)]
  rounded-3xl md:rounded-2xl flex flex-col justify-between
  md:transition-[width,padding] duration-200 ease-out`.
- **Collapsed (desktop)**: `md:w-12`. Logo button `md:h-9 md:w-9 h-10 w-10
  rounded-full md:hover:bg-foreground/10!` — hover shows PanelLeftOpen icon,
  tooltip "Expand navigation" (⌘+B also toggles).
- **Expanded**: `md:w-72 md:bg-muted/60 md:backdrop-blur-xl border-border
  shadow-md is-expanded`; close button (PanelLeftClose) top-right, tooltip
  "Close Sidebar".
- **Strip actions** (SidebarNavigationActions): New chat (SquarePen, ⌘⇧O),
  Search (⌘K), MCP Servers (McpLogo), Settings (gear) — each
  `w-full min-w-9 justify-between px-2 backdrop-blur-none!
  hover:[&>kbd]:opacity-100` with `ActionIcon` (h-9 w-9 rounded-full),
  tooltip `TooltipSide.RIGHT`, active state `bg-foreground/10
  text-foreground`, keyboard hint kbd `hidden md:inline-flex h-4 items-center
  rounded bg-foreground/10 px-1 text-[10px]` (opacity-0 → hover:opacity-100).
  Container `px-4 my-2 flex flex-col gap-5 md:gap-1`.
- **Conversation list** (SidebarNavigationConversationList): header rows
  `text-muted-foreground inline-flex h-8 shrink-0 items-center rounded-md
  px-2 text-xs font-medium` ("Pinned", "Recent conversations", "No
  conversations yet"); scroll area `min-h-0 flex-1 overflow-y-auto`.
- **Conversation item**: `group flex min-h-9 w-full cursor-pointer items-center
  justify-between space-x-3 rounded-lg py-1.5 text-left transition-colors
  hover:bg-foreground/10`; active `bg-accent text-accent-foreground`;
  inner: name `min-w-0 flex-1` + truncate, branch depth padding `pl-{depth*8}px`
  (FORK_TREE_DEPTH_PADDING 8 px), context-menu (⋮) on hover
  `opacity-0 group-hover:opacity-100`, pin 📌 `h-3.5 w-3.5`, rename dialog.
- **Search** (SidebarNavigationSearch): `mb-4 px-2` — uses SearchInput (§2.4).
- **Search results** (SidebarNavigationSearchResults): header + scroll list;
  result rows like conversation items; highlights in `bg-accent`.
- **Selection bar** (SidebarNavigationSelectionBar): `flex items-center
  gap-1.5 rounded-xl border border-border/50 bg-background/50 px-2 py-1.5
  shadow-sm backdrop-blur-xl` with count `truncate text-xs font-medium` +
  Select all / Delete (trash icon) buttons.

## 2.2 ActionIcon

`ActionIcon` — icon-only button: `flex h-6 w-6 p-0 hover:bg-transparent
data-[state=open]:bg-transparent!` (transparent hover), wraps icon
`h-4 w-4`; props: icon, size (sm/default/lg/icon-lg), tooltip, tooltipSide,
ariaLabel. Used for all icon buttons (message actions, sidebar, dialogs).

`ActionIconCopyToClipboard` — copy button: clipboard write + check icon
feedback (Copy → Check, 300 ms), 16 px icon, `text-muted-foreground`.

## 2.3 Misc chrome

- **Logo**: the Llama UI mark (inline SVG, 28 px in sidebar, gradient
  fill). Sits in the sidebar logo button; hover (desktop) swaps to
  PanelLeftOpen when collapsed.
- **TruncatedText**: `min-w-0` wrapper + `block truncate`; optional tooltip
  on hover with full text (`z-[9999]`, tooltip chrome).
- **HorizontalScrollCarousel**: `relative` container with prev/next arrow
  buttons `absolute top-1/2 left-4 z-10 flex h-6 w-6 -translate-y-1/2
  items-center justify-center rounded-full bg-background/25 shadow-md
  backdrop-blur-xs transition-opacity hover:bg-background/40`; content
  scrolls horizontally with snap.
- **KeyboardShortcutInfo**: `inline-flex items-center gap-1 rounded bg-foreground/10
  px-1.5 py-0.5 text-[10px] font-medium text-foreground/70`; variants
  (default/outline/danger); renders key caps like `⌘⇧O`.
- **ConversationSelection**: selection-mode wrapper `space-y-4` — header
  `flex items-center justify-between text-sm text-muted-foreground`,
  marquee-selectable list `overflow-hidden rounded-md border`, sticky
  header `sticky top-0 z-10 bg-muted`.
- **BadgeInfo**: info badge `inline-flex items-center gap-1 rounded-md bg-muted
  px-2 py-0.5 text-xs text-muted-foreground` (used in model cards).
- **BadgesModality**: modality badges for models — image/audio/video/text
  pills `h-5 gap-1 bg-* px-1.5 text-[10px]` + 12 px icon, colors per
  modality (see §11).

## 2.4 Forms

- **SearchInput**: `relative` — search icon `absolute top-1/2 left-3 z-10
  h-4 w-4 -translate-y-1/2 text-muted-foreground`, input `pl-9`, clear
  button `absolute top-1/2 right-3 -translate-y-1/2` (X, hover:bg-muted).
- **InputWithSuggestions**: input + suggestion dropdown
  `absolute top-full right-0 left-0 z-10 mt-1 max-h-32 overflow-y-auto
  rounded-lg border border-border/30 bg-popover p-1 shadow-md`; suggestion
  rows `rounded-md px-3 py-2 text-sm hover:bg-accent cursor-pointer`;
  label `mb-1 text-muted-foreground`, error `text-destructive`.
- **KeyValuePairs**: key-value editor — rows `mb-2 flex items-center
  justify-between`, add button `inline-flex cursor-pointer items-center
  gap-1 rounded-md px-1.5 py-1 text-xs text-muted-foreground hover:bg-accent
  hover:text-foreground` (Plus icon), remove per row.

## 2.5 Server states

- **ServerLoadingSplash**: full-screen centered loader — logo + spinner
  `size-6 animate-spin text-muted-foreground` + "Connecting to server…"
  (14 px muted); shown while `serverLoading()`.
- **ServerErrorSplash**: full-screen error state — `flex flex-col items-center
  justify-center gap-3 p-8` + destructive icon 32 px + title (16 px
  semibold) + description (14 px muted) + Retry button (default variant).
- **ServerStatus**: small status dot in the sidebar footer — `h-2 w-2
  rounded-full` green (ok) / amber (loading) / red (error) + label
  `text-[11px] text-muted-foreground`; shows server address + version.
