# 12 — Misc, server states, forms

## 12.1 Logo

- The Llama UI mark — inline SVG; in the sidebar it sits in the logo
  button (28 px); on hover (collapsed desktop) the button swaps to
  PanelLeftOpen. Used on: sidebar, splash screens, dialog headers.

## 12.2 TruncatedText

- `{className} min-w-0` wrapper → `block truncate`; with `tooltip` prop:
  renders a Tooltip (hover) showing the full text (`z-[9999]`).

## 12.3 HorizontalScrollCarousel

- `relative` + horizontal snap scroll; prev/next buttons
  `absolute top-1/2 z-10 flex h-6 w-6 -translate-y-1/2 items-center
  justify-center rounded-full bg-background/25 shadow-md backdrop-blur-xs
  transition-opacity hover:bg-background/40` (left-4 / right-4, Chevron
  icons 14 px); arrows fade out at scroll edges.

## 12.4 KeyboardShortcutInfo

- kbd chip: `inline-flex items-center gap-1 rounded bg-foreground/10
  px-1.5 py-0.5 text-[10px] font-medium text-foreground/70`; variants:
  default / outline (`border border-border`) / danger (`text-destructive`);
  separator dot `h-1 w-1 rounded-full bg-foreground/30`.

## 12.5 ConversationSelection

- Selection-mode chrome: header `flex items-center justify-between text-sm
  text-muted-foreground` (count + cancel), marquee list
  `overflow-hidden rounded-md border` with rows `h-100` scroll area;
  sticky header `sticky top-0 z-10 bg-muted`.

## 12.6 Server states (splash/status)

- **ServerLoadingSplash**: `fixed inset-0 z-50 flex flex-col items-center
  justify-center gap-4 bg-background` — Logo (48 px, pulsing), spinner
  `size-6 animate-spin text-muted-foreground`, text
  "Connecting to server…" `text-sm text-muted-foreground`.
- **ServerErrorSplash**: same shell — AlertTriangle 32 px
  `text-destructive`, title `text-base font-semibold` ("Server is not
  running or unreachable"), description `text-sm text-muted-foreground`
  (address + port), Retry button (default) + "Open settings" (outline).
- **ServerStatus** (sidebar footer strip): dot `h-2 w-2 rounded-full`
  (ok `bg-green-500` / loading `bg-amber-500 animate-pulse` / error
  `bg-red-500`) + `text-[11px] text-muted-foreground` label (server
  address · version).

## 12.7 Badges

- BadgeInfo: `inline-flex items-center gap-1 rounded-md bg-muted px-2 py-0.5
  text-xs text-muted-foreground` (icon 12 px + label).
- BadgesModality: modality pills — see §11.2.

## 12.8 ActionIcon / copy

- See §02.2 (ActionIcon, ActionIconCopyToClipboard).

## 12.9 Forms

- **SearchInput**: `relative` — icon `absolute top-1/2 left-3 z-10 h-4 w-4
  -translate-y-1/2 text-muted-foreground`, input `pl-9`, clear X
  `absolute top-1/2 right-3 -translate-y-1/2 h-4 w-4 text-muted-foreground
  hover:text-foreground` (shown when non-empty), keyboard hint ⌘K chip.
- **InputWithSuggestions**: label `mb-1 text-muted-foreground`; input +
  suggestions dropdown `absolute top-full right-0 left-0 z-10 mt-1 max-h-32
  overflow-y-auto rounded-lg border border-border/50 bg-background shadow-lg`
  with rows `w-full px-3 py-1.5 text-left text-sm hover:bg-accent`; error
  `text-destructive`.
- **KeyValuePairs**: rows `mb-2 flex items-center justify-between` (key
  input + value input + remove X), header
  `text-xs font-medium select-none` (label), add button
  `inline-flex cursor-pointer items-center gap-1 rounded-md px-1.5 py-1
  text-xs text-muted-foreground hover:bg-accent hover:text-foreground`.
