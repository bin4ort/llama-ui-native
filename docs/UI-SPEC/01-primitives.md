# 01 — UI Primitives (shared components)

All primitives follow the shadcn/bits-ui pattern. Shared constants:

```
INPUT_CLASSES  = bg-muted/60 dark:bg-muted/75
                 border border-border/30 focus-within:border-border
                 dark:border-border/20 dark:focus-within:border-border
                 shadow-sm outline-none text-foreground
BOX_BORDER     = border border-border/30 focus-within:border-border
                 dark:border-border/20 dark:focus-within:border-border
PANEL_CLASSES  = bg-background border border-border/30 dark:border-border/20
                 shadow-sm backdrop-blur-lg! rounded-t-lg!
ICON_CLASS_DEFAULT = 'h-4 w-4'
ICON_CLASS_SPIN    = 'h-4 w-4 animate-spin'
```

Full base-class strings and state behavior for every primitive are in
`../UI-SPEC.md` §3. This file lists what each primitive is used for and any
component-specific notes.

| Component | Purpose / notes |
|---|---|
| **Button** | 6 variants (default/outline/secondary/ghost/destructive/link) × 6 sizes (default h-9 / sm h-8 / lg h-10 / icon size-9 / icon-lg size-10 / icon-sm size-5). Base: `inline-flex shrink-0 items-center justify-center gap-2 whitespace-nowrap rounded-md text-sm font-medium outline-none transition-all focus-visible:ring-[3px] …`. |
| **Input** | `h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-base shadow-xs … md:text-sm dark:bg-input/30`. File inputs: `type="file"` variant with muted styling. |
| **Textarea** | `flex field-sizing-content min-h-16 w-full …` — auto-grows; 16→14 px at md+. |
| **Label** | `flex items-center gap-2 text-sm leading-none font-medium select-none …`. |
| **Checkbox** | 16×16, `rounded-[4px]`, check icon `size-3.5`; checked → primary bg + primary-foreground; indeterminate → minus. |
| **Select** | Trigger `h-9 w-full … dark:bg-input/30` + ChevronDown 16 px; content popover `z-[var(--layer-popover,1000000)] min-w-[8rem] rounded-md border bg-popover text-popover-foreground shadow-md` + zoom/slide animations. |
| **Dialog** | Overlay `fixed inset-0 z-50 bg-black/50` (fade). Content `fixed top-[50%] left-[50%] z-50 grid max-h-[100dvh] w-full max-w-[calc(100%-2rem)] -translate-x-1/2 -translate-y-1/2 gap-4 overflow-y-auto rounded-lg border border-border/30 bg-background p-6 shadow-lg sm:max-w-lg` + zoom-in/out. Title 18 px semibold, description 14 px muted. Close `rounded-xs opacity-70 hover:opacity-100`. |
| **AlertDialog** | Dialog chrome; destructive action for destructive flows. |
| **Sheet** | Side panel: `inset-y-0 right-0 h-full w-3/4 border-l … bg-background shadow-lg` + slide-in-from-side; overlay black/50. |
| **DropdownMenu** | Content `z-50 min-w-[8rem] rounded-md border border-border bg-popover p-1.5 text-popover-foreground shadow-md dark:border-border/20`; items `rounded-sm px-2 py-1.5 text-sm data-highlighted:bg-accent …`, destructive variant, inset `pl-8`, check/radio items with 16 px icons. |
| **Tooltip** | `rounded-md bg-primary px-3 py-1.5 text-xs text-primary-foreground` + arrow `size-2.5 rotate-45 rounded-[2px] bg-primary` positioned per side; 300 ms delay (app-level). |
| **Badge** | `inline-flex items-center justify-center gap-2 whitespace-nowrap rounded-md border px-2.5 py-0.5 text-xs font-semibold w-fit … [&>svg]:size-3 shrink-0` + variants (default/secondary/destructive/outline) + `backdrop-blur-sm`. |
| **Card** | `flex flex-col gap-6 rounded-xl bg-card py-6 text-card-foreground shadow-sm` + BOX_BORDER. Header/title/description/footer slots. |
| **Separator** | `shrink-0 bg-border` — `h-px w-full` horizontal / `w-px` vertical. |
| **Skeleton** | `animate-pulse rounded-md bg-accent`. |
| **Collapsible** | No chrome; trigger ghost + chevron `h-4 w-4 text-muted-foreground` (rotate-90 open). |
| **Switch** | `peer inline-flex h-5 w-9 shrink-0 … rounded-full border border-transparent bg-input data-[state=checked]:bg-primary …` thumb `size-4 rounded-full bg-background shadow-xs`. |
| **RadioGroup** | Item: `size-4 rounded-full border border-input … data-[state=checked]:border-primary` + inner dot `size-2 bg-primary`. |
| **ScrollArea** | Thin scrollbar (8 px, thumb `bg-border rounded-4px`). |
| **Table** | `w-full caption-bottom text-sm`; header `[&_tr]:border-b`; rows `border-b transition-colors hover:bg-muted/50`; cells `p-2 align-middle`. |
| **Popover / HoverCard** | Content popover chrome like select content; side slide-ins. |
| **Alert** | `rounded-lg border px-4 py-3 text-sm` + variants (default/info/success/warning/destructive) with icon 16 px. |
| **ButtonGroup** | Inline segmented buttons; joined rounded corners (`rounded-l-md`/`rounded-r-md` on edges). |
| **Empty** | `flex flex-col items-center justify-center gap-2 py-8 text-muted-foreground` + icon 32 px + text sm. |
| **utils (cn)** | Class merge helper (clsx + tailwind-merge) used everywhere. |
