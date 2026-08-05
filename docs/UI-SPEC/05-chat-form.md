# 05 — Chat Form (composer)

## 5.1 ChatForm (composer container)

- Floating bar above the chat, `max-w-3xl mx-auto`:
  `flex-column relative min-h-12 items-center rounded-4xl md:rounded-3xl
  py-2 pb-2.25 shadow-sm transition-all focus-within:shadow-md md:py-3!`
  (mobile radius 32 px, desktop 24 px; shadow-sm → shadow-md on focus-within).
- Disabled (streaming lock): `disabled:opacity-70`.
- Textarea: `{INPUT_CLASSES} overflow-hidden rounded-4xl md:rounded-3xl
  backdrop-blur-md` — auto-resize `field-sizing-content`, 14 px text,
  placeholder `Message…`; IME guard.
- Action rows: top row `px-3` (add menu + right-side actions), bottom row
  `px-5 py-1.5 md:pt-0` (tools/attachments area).
- Above-bar: ChatAttachments list + ChatFormContextGauge.

## 5.2 ChatFormActions (bars)

- `flex w-full items-center gap-3` — left cluster `mr-auto flex items-center
  gap-2` (Add menu, persona preset, models, tools, record), right cluster
  `flex items-center gap-1.5` (context gauge, submit).
- Buttons: `group h-8 w-8 rounded-full p-0` icon buttons with
  `stroke-muted-foreground group-hover:stroke-foreground` icons (16 px).

## 5.3 Add menu (ChatFormActionAdd)

- Trigger: `file-upload-button md:h-8 md:w-8 h-9 w-9 rounded-full p-0`
  (Plus icon); opens DropdownMenu (desktop, `w-52`) or Sheet (mobile,
  `max-h-[85vh] gap-0 overflow-y-auto`).
- Items: `flex cursor-pointer items-center gap-2` + 16 px icon + label 14 px;
  disabled items `cursor-default opacity-50` with tooltip
  ("Requires a vision model", etc.). Item groups: Images, Audio Files,
  Video Files, Text Files, PDF Files, System message, MCP prompts,
  MCP resources, Tools.
- Submenus:
  - MCP servers submenu `w-72 pt-0`: server rows
    `flex w-full items-center justify-between gap-2 rounded-sm px-2 py-2
    text-left transition-colors hover:bg-accent` with logo + name + check;
    "Configure servers…" link.
  - Tools submenu `w-72 p-0`: per-tool toggles with Switch; "Always allow"
    indicator; empty state `px-3 py-4 text-center text-sm text-muted-foreground`.
  - Reasoning submenu `w-60`: thinking on/off + effort options
    (`text-sm inline-flex gap-2`, check icon; active `text-amber-400`).

## 5.4 Persona preset (ChatFormActionPreset)

- Trigger: `h-8 cursor-pointer gap-1 px-2 text-xs` — Sparkles/BadgeCheck
  icon 14 px (`text-amber-400` when a persona is active, else
  `text-muted-foreground`), label `min-w-0 max-w-24 truncate`
  (preset name or "Default"), ChevronDown 12 px.
- Dropdown `w-64`: "Prompt presets" label; Default item
  `flex cursor-pointer items-center justify-between gap-2` with Check 16 px;
  favorites list (star + name, Check on active); "All presets…" item.

## 5.5 Models (ChatFormActionModels)

- Trigger: compact selector — model id + badges + ChevronDown; opens
  ModelsSelectorDropdown/Sheet (§11); loading spinner
  `h-3.5 w-3.5 animate-spin`.

## 5.6 Record & submit

- Record: `h-8 w-8 rounded-full p-0` mic button; recording state
  `animate-pulse fill-white` red dot; tooltip "Record audio".
- Submit: icon button `h-12 w-12` (ArrowUp in primary circle, 20 px) —
  disabled when empty/streaming; Enter sends (Shift+Enter newline).

## 5.7 Context gauge (ChatFormContextGauge)

- Trigger: dial `flex h-5 w-5 cursor-default items-center justify-center` —
  SVG ring (ContextGaugeDial, `transition-colors duration-300` stroke color
  by level: green <70 %, amber 70–90 %, red >90 %).
- Popup (ContextGaugePopup): `absolute z-50 w-64 -translate-x-1/2 rounded-lg
  border border-border/50 bg-popover p-3 text-sm text-popover-foreground
  shadow-lg ring-1 ring-foreground/10` — title "Context usage" + rows
  (prompt / history / estimated completion) `flex items-center justify-between
  font-mono text-xs`, details sections `mt-3 border-t border-border/50 pt-4`
  with uppercase labels `text-[11px] font-medium uppercase tracking-wide
  text-muted-foreground/70 mb-2`.
- Load-model hint (when context too full): `flex flex-col gap-2 border-t
  border-border/50 pt-2 text-xs text-muted-foreground` + "Load model" button
  (`self-start`).

## 5.8 Pickers (system prompt / MCP)

- ChatFormPickers (system prompt editor dialog): textarea + `INPUT_CLASSES`,
  footer Cancel/Save; "Show system message in conversations" switch.
- ChatFormPickerMcpPrompts: `p-4` — prompt list (server → templates),
  argument form `space-y-3 pt-4` with argument inputs
  (ChatFormPromptPickerArgumentInput: label `mb-1 text-muted-foreground`,
  error `text-destructive`, suggestion dropdown `absolute top-full right-0
  left-0 z-10 mt-1 max-h-32 overflow-y-auto rounded-lg border
  border-border/50 bg-background shadow-lg`), footer
  `mt-8 flex justify-end gap-2` (Cancel/Send prompt).
- ChatFormPickerMcpResources: resource rows with type chip
  `inline-flex items-center rounded-full bg-primary/10 px-1.5 py-0.5
  text-[10px] font-medium text-primary`, loading/error states, "Use
  resource" button.
