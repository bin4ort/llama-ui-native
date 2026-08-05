# 11 — Models

## 11.1 ModelId

- `font-medium` + optional truncate: `flex min-w-0 flex-wrap items-center
  gap-1` — short label (e.g. "Qwen2.5-VL") `truncate font-medium`; when
  `showRawModelNames`, the full path is shown instead.

## 11.2 ModelBadge / BadgesModality

- ModelBadge: icon 12 px + label pill `inline-flex items-center gap-1
  rounded-md bg-muted px-1.5 py-0.5 text-[10px] text-muted-foreground`.
- Modality badges: Images `bg-green-*`, Audio `bg-blue-*`, Video
  `bg-purple-*`, Text `bg-muted` — `h-5 gap-1 px-1.5 text-[10px]`.
- Quantization/tags: Badge component variants (§01) — e.g. Q8_0, Q4_K_M,
  "vision", "reasoning".

## 11.3 ModelsSelectorOption

- Row in dropdown/sheet lists:
  `flex min-w-0 items-center gap-2 rounded-md px-2 py-1.5 text-sm
  hover:bg-accent data-[highlighted]:bg-accent` — radio check
  `flex shrink-0 items-center gap-1` (Check 14 px, active) + ModelId +
  badges + meta column `text-xs text-muted-foreground` (context, size);
  favorite star + "always allow" icon on hover
  `pointer-events-none flex items-center justify-center gap-0.75 pl-2
  opacity-0 group-hover:pointer-events-auto group-hover:opacity-100`.

## 11.4 ModelsSelectorList

- Scrollable list `max-h-80 overflow-y-auto p-1`; section labels
  `px-2 py-1 text-xs font-medium uppercase tracking-wide text-muted-foreground`
  (favorites / recent / all); loading skeleton rows.

## 11.5 ModelsSelectorDropdown

- Trigger (chat bar): `flex items-center gap-2 text-xs text-muted-foreground`
  — model label `min-w-0 overflow-hidden` + badges + ChevronDown 14 px;
  loading `h-3.5 w-3.5 animate-spin`.
- Dropdown content (Select primitive, `w-80`): search input, option list,
  footer "Manage models…" link + "Refresh" button.

## 11.6 ModelsSelectorSheet (mobile)

- Sheet from bottom `max-h-[85vh]` — header `sticky top-0 z-10 bg-background
  px-4 py-3 border-b border-border/30` (title "Model" + close), option
  list, footer note.

## 11.7 ModelLoadHighlight

- Loading progress bar on the active model option:
  `pointer-events-none absolute inset-x-0 bottom-0 h-0.5 overflow-hidden
  rounded-b-sm` → inner `h-full animate-pulse bg-primary transition-[width]
  duration-200 ease-out` (width = load progress %).

## 11.8 ModelInformation dialog

See §08 (ModelInformation): fields shown as label/value rows
(`w-[10rem]` label column) — name, id, quantization, context length,
tags, file size, owner; action "Select model" (primary) + Close.
