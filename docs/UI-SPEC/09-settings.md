# 09 — Settings

## 9.1 Page shell (SettingsChat)

- Wrapper: `mx-auto flex h-full w-full flex-col md:pl-8`.
- Body: `flex flex-1 flex-col gap-4 md:flex-row` → SettingsChatDesktopSidebar
  + content.
- Content: `mx-auto max-w-3xl flex-1` → `space-y-6 p-4 md:p-6 md:pt-28`.
  - Section title row: `mb-6 flex items-center gap-2 border-b border-border/30
    pb-6 md:flex` — icon `h-5 w-5` + `h3.text-lg font-semibold`.
  - Groups: `space-y-6`.
  - Reload hint (when settings came from the server): "Reload app" outline
    button.
  - Footer note: `mt-8 border-t border-border/30 pt-6` → `p.text-xs
    text-muted-foreground` "Settings are saved in browser's localStorage".

## 9.2 SettingsChatDesktopSidebar (left nav)

- `sticky top-2 hidden w-64 flex-col self-start bg-background py-4 md:flex
  gap-6`.
- Header: `flex items-center gap-2 py-2` — Settings icon `h-5 w-5 md:h-6
  md:w-6` + `h1.text-xl font-semibold md:text-2xl` "Settings".
- Nav: `space-y-1` — items `flex w-full cursor-pointer items-center gap-3
  rounded-lg px-3 py-2 text-left text-sm no-underline transition-colors
  hover:bg-accent`; active `bg-accent text-accent-foreground`, inactive
  `text-muted-foreground`; icon 16 px + `span.ml-2` label.
- Footer: `mt-auto pt-4 text-center opacity-50 text-[11px]` version line
  `Llama UI Native v… (build …)`.

## 9.3 SettingsChatMobileHeader

- Mobile top bar: `sticky top-0 z-10 flex items-center justify-between
  bg-background/80 px-4 py-2 backdrop-blur` — back button, current section
  title `text-sm font-semibold`, section dropdown (Settings icon trigger →
  sheet with the same nav list).

## 9.4 SettingsFooter (bottom action bar)

- `sticky bottom-0 mx-auto mt-4 flex w-full justify-between p-6`:
  Reset to default (outline button + RotateCcw 12 px) + Save settings
  (primary button). Reset opens AlertDialog:
  title "Reset Settings to Default", description (full reset explanation).

## 9.5 SettingsGroup

- `<div><h3 class="mb-6 text-base font-semibold">Title</h3><div
  class="space-y-8">fields</div></div>` — groups contain the field list.

## 9.6 Fields (SettingsChatFields)

Registry-driven (§3.16 of ../UI-SPEC.md):
- Label row `flex items-center gap-2` + `label.flex items-center gap-1.5
  text-sm font-medium` (+ FlaskConical 14 px for experimental).
- Parameter-source indicator dot when value ≠ server default.
- Inputs: INPUT_CLASSES (`bg-muted/60 … shadow-sm … px-3 py-2 text-sm`),
  `Default: …` placeholder from /props, reset ↺ button
  `absolute top-1/2 right-2 h-5 w-5 … hover:bg-muted` when customized.
- Selects: native select with Select-primitive chrome.
- Checkboxes: primitive checkbox + label `cursor-pointer text-sm font-normal`.
- Help: `mt-1 text-xs text-muted-foreground`.
- Spacing: `py-3` per field, `space-y-1.5`; groups `space-y-8`.

## 9.7 Section-specific components

- **SettingsChatSystemPromptEditor**: textarea `min-h-40` + INPUT_CLASSES +
  "Show system message in conversations" checkbox row.
- **SettingsChatSystemPromptPresets**: search input + tool-table-style list
  (header row "Preset | Favorite | Edit | Delete" `text-xs
  text-muted-foreground`, rows with star ★/☆, truncated name+description,
  icon controls Edit/Delete with tooltips, inline edit panel with
  name/description/content fields); "Create with wizard…" primary button.
- **SettingsChatToolTable / BuiltinToolTable**: tool tables — header
  `text-xs font-medium text-muted-foreground`, rows
  `flex items-center gap-3 border-b border-border/30 py-2` (icon 16 px,
  name `text-sm font-medium`, description `text-xs text-muted-foreground`,
  enabled Switch, "Always allow" star toggle `text-amber-400`).
- **SettingsChatParameterSourceIndicator**: amber dot 8 px with tooltip
  "Custom value (overrides server default)".
- **SettingsChatSeparator**: `my-2 h-px bg-border/50`.
- **SettingsChatLinkButton**: ghost link button `text-primary underline
  underline-offset-4 hover:underline text-sm`.

## 9.8 Settings sections content

Each section = a group list per registry entry (see `registry.js` port in
the vanilla app + `../UI-SPEC.md` §2 for exact labels/help):
General (language, theme, model select with favorites, system message),
Display (12 checkboxes + custom JSON/CSS textareas), Sampling (12 numeric
inputs), Penalties (8 numeric inputs), Tools (10 tool toggles + always-allow
staging), Agentic (max turns, MCP timeout, preset tools, title generation),
Developer (API key, endpoint, sandbox, Web UI, log level, reset), Import/Export.
