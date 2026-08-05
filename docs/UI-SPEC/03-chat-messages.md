# 03 — Chat Messages

## 3.1 ChatMessages (list)

- Container `mx-auto mt-12 w-full max-w-3xl` (`md:` variant max-w-[48rem] for
  conversation mode) + `space-y-6`; renders `activeMessages` in order;
  branch depth padding via `FORK_TREE_DEPTH_PADDING` (8 px) on nested
  branches.

## 3.2 ChatMessage (dispatcher)

- `class="chat-message"`; picks the renderer by `message.type`:
  system / persona → ChatMessageSystem, MCP prompt → ChatMessageMcpPrompt,
  user → ChatMessageUser, else assistant.
- Holds per-message edit context (edit form + "Show system message" flow).

## 3.3 ChatMessageSystem (system + persona rows)

- Row: `group flex flex-col items-end gap-3 md:gap-2` + `system-edit-active`
  while editing.
- Bubble: `relative max-w-[80%]` → inner
  `overflow-y-auto rounded-[1.125rem] !border-2 !border-dashed
  !border-border/50 bg-muted px-3.75 py-1.5 data-[multiline]:py-2.5`
  (18 px radius, dashed border, muted bg); content
  `markdown-system-content` (MarkdownContent).
- Edit mode: textarea `min-h-[60px] w-full resize-none rounded-2xl px-3 py-2
  text-sm` + INPUT_CLASSES; footer `mt-2 flex justify-end gap-2` with
  Cancel (ghost, h-8 px-3) + Save (default h-8 px-3); "expand" hint with
  bottom fade `pointer-events-none absolute right-0 bottom-0 left-0 h-48
  bg-gradient-to-t from-muted to-transparent`.
- Centered label above: `mb-2 flex justify-center` — "System message" /
  persona name pill `h-8 px-3` (badge, muted).
- Footer actions row (ChatMessageActionIcons, see §3.9).

## 3.4 ChatMessageUser

- Row: `chat-message-user group flex flex-col items-end gap-3 md:gap-2`.
- Bubble (ChatMessageUserBubble): `chat-message-user-bubble max-w-[80%]
  overflow-y-auto rounded-[1.125rem] border-none bg-primary/5 px-3.75 py-1.5
  backdrop-blur-md data-multiline:py-2.5` + theme-tinted text
  (`textColorClass`); content `markdown-user-content` rendered with
  MarkdownContent when `renderUserContentAsMarkdown`, else
  `text-md whitespace-pre-wrap`.
- Info row: `info my-2 grid w-full justify-items-end gap-4 tabular-nums`
  (message stats, see §3.8).

## 3.5 ChatMessageAssistant

- Wrapper: `chat-message-assistant text-md group w-full leading-7.5`.
- Contains: model selector line (ChatMessageAssistantModel —
  ModelsSelectorDropdown + ModelBadge), processing info
  (`shimmer-text text-sm` while streaming, `flex flex-col items-start
  gap-2`), reasoning block, agentic content (`agentic-text` +
  `agentic-content gap-2`, turns `agentic-turn group/turn grid gap-2` with
  `turn-stats`), markdown content, action icons, statistics
  (`info my-6 grid gap-4 tabular-nums`).
- Raw output mode: `pre.raw-output` plain text.

## 3.6 Reasoning block

- `my-2` wrapper; content `reasoning-content text-[13px] leading-relaxed
  wrap-break-word whitespace-pre-wrap text-muted-foreground`; rendered as
  CollapsibleContentBlock (see §07) with a "Thought" toggle; spinner state
  while `showThoughtInProgress`.

## 3.7 Tool-call rendering

- **ToolCallBlock**: `my-2` wrapper — header row `mb-2 flex items-center
  gap-2 text-xs text-muted-foreground/70` (tool name `font-mono`, spinning
  Loader2 12 px while running, error/status chips).
- **Default**: args `rounded bg-muted/20 p-2 text-xs text-muted-foreground/70
  italic`; output via SyntaxHighlightedCode; error chip
  `rounded bg-yellow-500/10 p-2 text-xs text-yellow-600 italic
  dark:text-yellow-400`.
- **Edit/Write file**: filename `font-mono`, error
  `flex items-start gap-2 rounded bg-red-500/10 p-2 text-xs text-red-600
  italic dark:text-red-400` with XCircle 12 px, diff output class
  `diff-…`, result summary `text-xs text-muted-foreground/70 italic`.
- **Exec shell**: terminal output `terminal-output font-mono text-[11px]
  leading-relaxed whitespace-pre-wrap`; status icons Check (green) /
  AlertTriangle; error `text-red-600 italic`.
- **Read file / glob / grep search**: file path `font-mono
  text-muted-foreground`; results `max-h-96 overflow-auto font-mono text-xs`;
  match lines with `bg-yellow-500/20` highlights.
- **Search results**: pill chips `inline-flex max-w-full items-center gap-1.5
  rounded-full border bg-muted px-2.5 py-1 text-xs transition-colors
  outline-none hover:bg-muted/80 focus-visible:ring-2 focus-visible:ring-ring`
  with favicon `h-3 w-3 shrink-0 rounded-sm` + Globe icon; hover card shows
  page text.
- **Get datetime**: `flex items-center gap-2 py-1.5` with Clock icon 14 px
  muted + `font-mono text-foreground/80 text-sm font-medium` value.
- **Run JavaScript**: code via SyntaxHighlightedCode + Terminal output;
  error chip standard.
- Permission requests: **ChatMessageActionCardPermissionRequest** —
  `overflow-hidden rounded-md shadow-sm` with ButtonGroup
  (`!rounded-r-none` allow-once + "Always allow" dropdown
  `min-w-[8rem]`) + Deny (destructive).

## 3.8 Statistics

- ChatMessageStatistics: toggle `inline-flex h-5 w-5 items-center
  justify-center rounded-sm` (active `bg-muted-foreground/15`), views:
  tokens / time / t/s badges `inline-flex items-center rounded-sm
  bg-muted-foreground/15 p-0.5` + per-stat tooltips; values
  `text-xs text-muted-foreground tabular-nums`. Badges via BadgeInfo
  (icon 12 px + label).

## 3.9 Action icons

- ChatMessageActionIcons: `pointer-events-auto inset-0 flex items-center
  gap-1 opacity-100 transition-all duration-150` positioned by
  `actionsPosition` (inline / overlay); actions: copy (ActionIconCopyToClipboard),
  regenerate, edit (pencil), delete (trash), continue (↓ "Continue"
  `text-destructive hover:text-destructive`), branching controls
  `flex items-center gap-1 text-xs text-muted-foreground` (◀ ▶
  `h-5 w-5 p-0`, disabled `opacity-40`, breadcrumb `px-1 font-mono text-xs`).
- Overlay mode: opacity-0 → `group-hover:opacity-100`.
- ActionCard wrapper: `my-2 rounded-lg border border-border bg-card p-3`
  with header `mb-3 flex items-center gap-2 text-sm` (icon 16 px muted).

## 3.10 MCP prompt message

- Row like user (`flex flex-col items-end gap-3 md:gap-2`); content
  `rounded-sm bg-purple-200/60 px-1.5 py-0.5 text-xs` purple chip + server
  name + template title + args (TruncatedText); rendered in a Card
  `w-full max-w-[80%]`.
