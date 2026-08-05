# Llama UI Native — UI & Component Specification

Derived from the v0.4.3 Svelte sources (recovered from git history) as the
exact, implementation-independent instruction set for the interface. This is
the single source of truth for how every element looks and behaves in every
theme — for the current vanilla web frontend AND for any future native
(web-wrapper-free) implementation.

Live implementation status: design tokens ported into
`web/kernel/tokens.css`; structural port of settings shell + sidebar done;
component-level port of the shadcn-style primitives pending (see §5).

---

## 1. Design tokens (global)

### 1.1 Radius scale (Tailwind v4 computed from `--radius: 0.625rem`)

| Token | Value | Usage |
|---|---|---|
| `rounded-xs` | 0.125rem (2 px) | small inner elements (dialog close button, checkbox indicators) |
| `rounded-sm` | 0.25rem (4 px) | dropdown items, icon-sm buttons, checkbox (`rounded-[4px]` = 4 px) |
| `rounded-md` | 0.375rem (6 px) | buttons, inputs, textareas, select trigger, tooltip, skeleton |
| `rounded-lg` | 0.5rem (8 px) | dialogs, alert dialogs, size-lg buttons |
| `rounded-xl` | 0.75rem (12 px) | cards |
| `rounded-2xl` | 1rem (16 px) | sidebar (md), icon strip |
| `rounded-3xl` | 1.5rem (24 px) | sidebar (mobile), composer (`rounded-4xl` mobile = 2rem / 32 px) |
| `rounded-[4px]` | 4 px fixed | checkbox |

### 1.2 Shadows

| Token | Value | Usage |
|---|---|---|
| `shadow-xs` | 0 1px 2px 0 rgb(0 0 0 / 0.05) | checkbox, inputs |
| `shadow-sm` | 0 1px 3px 0 rgb(0 0 0 / 0.1), 0 1px 2px -1px rgb(0 0 0 / 0.1) | buttons, cards, composer (rest state) |
| `shadow-md` | 0 4px 6px -1px rgb(0 0 0 / 0.1), 0 2px 4px -2px rgb(0 0 0 / 0.1) | popovers, dropdowns, select content, composer (focus-within) |
| `shadow-lg` | 0 10px 15px -3px rgb(0 0 0 / 0.1), 0 4px 6px -4px rgb(0 0 0 / 0.1) | dialogs |

### 1.3 Fonts

- `--font-sans`: `ui-sans-serif, system-ui, sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji"` — overridden by the bundled Noto Sans stack (Latin/Cyrillic/Greek/Vietnamese + Noto Sans SC/JP/KR for CJK, per `html:lang`)
- `--font-mono`: `ui-monospace, SFMono-Regular, 'SF Mono', Monaco, 'Cascadia Code', 'Roboto Mono', Consolas, 'Liberation Mono', Menlo, monospace`
- Base size: 14 px (body); headings/scale per component spec

### 1.4 Z-layers

| Layer | Value |
|---|---|
| popover layer (select/dropdown/sheet) | `--layer-popover: 1000000` |
| modal (dialog overlay + content) | `z-50` |
| sidebar | `z-10` |
| tooltip | `z-50` |

### 1.5 Chat-specific layout vars (used by chat screen geometry)

```
--chat-form-area-height: 8rem (mobile) / 24rem (sm+)
--chat-form-area-offset: 2rem (mobile) / 12rem (sm+)
--chat-form-padding-top: 6rem
--max-message-height: max(24rem, min(80dvh, calc(100dvh - var(--chat-form-area-height) - 12rem)))
```

### 1.6 Global state conventions (apply to ALL interactive elements)

- Focus-visible ring: `focus-visible:ring-[3px] focus-visible:ring-ring/50 focus-visible:border-ring`
- Disabled: `disabled:pointer-events-none disabled:opacity-50`
- Aria-invalid: `aria-invalid:border-destructive aria-invalid:ring-destructive/20` (`dark:aria-invalid:ring-destructive/40`)
- Dark-mode variance: `@custom-variant dark (&:is(.dark *))` — `dark:` utilities apply under `.dark` on any ancestor
- Icons inside buttons: `[&_svg:not([class*='size-'])]:size-4 [&_svg]:pointer-events-none [&_svg]:shrink-0` (16 px, non-interactive, shrink-proof)
- Hover on transparent surfaces: `hover:bg-muted-foreground/10 backdrop-blur-sm`

---

## 2. Color system (exact, per theme)

All colors are **oklch** CSS custom properties on `:root` (light base),
`.dark`, and the custom palette classes (`.nord`, `.dracula`, `.gruvbox`,
`.amoled`, `.snow`, `.gruvbox-light`, `.ionized-purple`, `.ionized-red`,
`.ionized-cyan`, plus base `.cobalt`/`.solarized` variants under `.dark`).
The full tables live in `web/kernel/tokens.css` (generated from the v0.4.3
`src/app.css`); the two base themes and every palette delta:

### 2.1 Light (`:root`)

| Token | oklch |
|---|---|
| background | 1 0 0 |
| foreground | 0.145 0 0 |
| card / popover | 1 0 0 (fg 0.145 0 0) |
| primary | 0.205 0 0 (fg 0.985 0 0) |
| secondary | 0.95 0 0 (fg 0.205 0 0) |
| muted | 0.97 0 0 (fg 0.556 0 0) |
| accent | 0.95 0 0 (fg 0.205 0 0) |
| destructive | 0.577 0.245 27.325 |
| border | 0.875 0 0 |
| input | 0.92 0 0 |
| ring | 0.708 0 0 |
| sidebar | 0.985 0 0 (fg 0.145 0 0; accent 0.97 0 0 / fg 0.205 0 0; border 0.922 0 0) |
| code-background | 0.985 0 0 (fg 0.145 0 0) |

### 2.2 Dark (`.dark`)

| Token | oklch |
|---|---|
| background | 0.16 0 0 |
| foreground | 0.985 0 0 |
| card / popover | 0.205 0 0 (fg 0.985 0 0) |
| primary | 0.922 0 0 (fg 0.205 0 0) |
| secondary | 0.29 0 0 (fg 0.985 0 0) |
| muted | 0.269 0 0 (fg 0.708 0 0) |
| accent | 0.269 0 0 (fg 0.985 0 0) |
| destructive | 0.704 0.191 22.216 |
| border / input | 1 0 0 / 30% |
| ring | 0.556 0 0 |
| sidebar | 0.2 0 0 (fg 0.985 0 0; accent 0.269 0 0 / fg 0.985 0 0; border 1 0 0 / 10%) |
| code-background | 0.225 0 0 (fg 0.875 0 0) |

### 2.3 Palette deltas (override the dark/light base; unlisted tokens inherit)

**nord** (dark): bg 0.25 0.01 252 · fg 0.88 0.02 240 · card 0.27 0.01 252 ·
primary 0.80 0.04 225 (fg white) · accent 0.65 0.07 195 (fg white) ·
muted-fg 0.6 0.03 240 · border/input 0.88 0.02 240 / 15% · ring 0.65 0.07 195 ·
code-bg 0.22 0.01 252 (fg 0.85 0.02 240)

**dracula**: bg 0.22 0.02 290 · fg 0.96 0.01 290 · card 0.24 0.02 290 ·
primary 0.8 0.1 315 (fg white) · accent 0.7 0.15 315 (fg white) · muted-fg
0.55 0.03 280 · border/input 0.96 0.01 290 / 15% · ring 0.7 0.15 315 ·
code-bg 0.2 0.02 290 (fg 0.9 0.01 290)

**cobalt**: bg 0.22 0.025 255 · fg 0.92 0.01 255 · card 0.24 0.025 255 ·
primary 0.78 0.1 75 (fg white) · accent 0.72 0.14 75 (fg white) ·
border/input 0.92 0.01 255 / 10% · ring 0.72 0.14 75 · code-bg 0.2 0.025 255

**solarized**: bg 0.24 0.02 205 · fg 0.78 0.03 190 · card 0.26 0.02 205 ·
primary 0.55 0.1 185 (fg 0.97 0.01 100) · accent 0.68 0.15 55 (fg white) ·
muted-fg 0.62 0.04 80 · border/input 0.78 0.03 190 / 12% · ring 0.68 0.15 55 ·
code-bg 0.22 0.02 205 (fg 0.7 0.03 120)

**gruvbox**: bg 0.23 0.02 85 · fg 0.88 0.04 85 · card 0.25 0.02 85 ·
primary 0.78 0.12 80 (fg white) · accent 0.75 0.14 85 (fg white) ·
muted 0.4 0.02 80 · border/input 0.88 0.04 85 / 15% · ring 0.75 0.14 85 ·
code-bg 0.21 0.02 85

**amoled**: bg 0.08 0 0 (true black) · fg 0.96 0 0 · card 0.12 0 0 ·
primary 0.88 0 0 (fg 0.12 0 0) · accent 0.18 0 0 (fg 0.96 0 0) ·
muted-fg 0.62 0 0 · border/input 0.96 0 0 / 15% · ring 0.45 0 0 ·
code-bg 0.1 0 0 (fg 0.82 0 0)

**snow** (light): bg 0.97 0.02 70 · fg 0.2 0.03 70 · card 0.99 0.01 70 ·
primary 0.38 0.06 65 (fg 0.98 0.02 70) · accent 0.9 0.04 60 (fg 0.25 0.05 70) ·
muted 0.94 0.03 68 · border/input 0.2 0.03 70 / 12% · ring 0.38 0.06 65 ·
code-bg 0.96 0.02 70

**gruvbox-light**: bg 0.95 0.03 85 · fg 0.22 0.04 85 · card 0.97 0.02 85 ·
primary 0.45 0.08 70 (fg 0.97 0.02 85) · accent 0.85 0.06 70 (fg 0.25 0.05 85) ·
border/input 0.22 0.04 85 / 12% · ring 0.45 0.08 70

**ionized-purple / -red / -cyan** (dark base; only hue-bearing tokens change):
- purple: primary 0.55 0.12 315 · accent 0.5 0.15 315 · muted-fg 0.65 0.06 315 ·
  border/input 0.45 0.12 315 / 16% · ring 0.5 0.15 315 · code-fg 0.78 0.04 315
- red: primary 0.52 0.12 22 · accent 0.48 0.16 20 · muted-fg 0.62 0.06 20 ·
  border/input 0.42 0.12 20 / 16% · ring 0.48 0.16 20 · code-fg 0.78 0.04 20
- cyan: primary 0.58 0.12 245 · accent 0.52 0.15 245 · muted-fg 0.65 0.06 245 ·
  border/input 0.45 0.12 245 / 16% · ring 0.52 0.15 245 · code-fg 0.78 0.06 245

Destructive for all dark-family palettes: `0.6 0.22 340` (ionized) or
`0.55–0.60 0.15–0.2 15–25` (theme hue).

**Palette → mode mapping** (for the theme selector): dark-family = amoled,
nord, dracula, cobalt, solarized, gruvbox, ionized-purple/red/cyan;
light-family = snow, gruvbox-light.

---

## 3. Component specs (exact)

Conventions: every component is the shadcn/bits-ui pattern — structure +
exact class string + state behavior + i18n. "Text" values are dictionary
keys (see §4).

### 3.1 Button

Structure: `<button type="button">` (+ optional inline SVG icon).

Base (all variants):
`inline-flex shrink-0 items-center justify-center gap-2 whitespace-nowrap rounded-md text-sm font-medium outline-none transition-all focus-visible:ring-[3px] focus-visible:ring-ring/50 focus-visible:border-ring disabled:pointer-events-none disabled:opacity-50`

Variants (exact):
- **default**: `bg-primary text-primary-foreground shadow-sm hover:bg-primary/90`
- **outline**: `shadow-sm hover:text-accent-foreground hover:bg-muted-foreground/10 backdrop-blur-sm dark:border-input border`
- **secondary**: `bg-background dark:bg-muted-foreground/15 dark:text-secondary-foreground shadow-sm text-foreground hover:bg-muted-foreground/20 dark:hover:bg-muted-foreground/25`
- **ghost**: `hover:text-accent-foreground hover:bg-muted-foreground/10 backdrop-blur-sm`
- **destructive**: `bg-destructive shadow-sm hover:bg-destructive/90 text-white!`
- **link**: `text-primary underline-offset-4 hover:underline`

Sizes:
- **default**: `h-9 px-4 py-2 has-[>svg]:px-3` (36 px tall, 16 px x-padding; 12 px when icon-only)
- **sm**: `h-8 gap-1.5 rounded-md px-3 has-[>svg]:px-2.5` (32 px)
- **lg**: `h-10 rounded-lg px-6 has-[>svg]:px-4` (40 px)
- **icon**: `size-9` (36×36) · **icon-lg**: `size-10` · **icon-sm**: `size-5 rounded-sm`

Behavior: transition-all 150 ms default; hover bg-alpha on primary/90;
focus ring 3 px at `ring/50` + border `ring`.

### 3.2 Input

Structure: `<input data-slot="input">`.

`flex h-9 w-full rounded-md border border-input bg-transparent px-3 py-1 text-base shadow-xs transition-[color,box-shadow] outline-none placeholder:text-muted-foreground focus-visible:border-ring focus-visible:ring-[3px] focus-visible:ring-ring/50 disabled:cursor-not-allowed disabled:opacity-50 aria-invalid:border-destructive aria-invalid:ring-destructive/20 md:text-sm dark:bg-input/30 dark:aria-invalid:ring-destructive/40`

Behavior: 36 px tall; transparent bg over `dark:bg-input/30`; placeholder
`muted-foreground`; 16 px text on mobile, 14 px at md+.

### 3.3 Textarea

`flex field-sizing-content min-h-16 w-full rounded-md border border-input bg-transparent px-3 py-2 text-base shadow-xs transition-[color,box-shadow] outline-none placeholder:text-muted-foreground focus-visible:border-ring focus-visible:ring-[3px] focus-visible:ring-ring/50 disabled:cursor-not-allowed disabled:opacity-50 aria-invalid:border-destructive aria-invalid:ring-destructive/20 md:text-sm dark:bg-input/30 dark:aria-invalid:ring-destructive/40`

Behavior: grows with content (`field-sizing-content`); min 64 px; 16→14 px
text at md+. Settings long text: `min-h-[10rem] w-full md:max-w-3xl`.

### 3.4 Label

`flex items-center gap-2 text-sm leading-none font-medium select-none group-data-[disabled=true]:pointer-events-none group-data-[disabled=true]:opacity-50 peer-disabled:cursor-not-allowed peer-disabled:opacity-50`

Behavior: 14 px, medium weight, no line-height gap. Settings field labels:
`flex items-center gap-1.5 text-sm font-medium` (14 px, 6 px gap, optional
experimental flask icon 14 px).

### 3.5 Checkbox

`peer flex size-4 shrink-0 items-center justify-center rounded-[4px] border border-input bg-background shadow-xs transition-shadow outline-none focus-visible:border-ring focus-visible:ring-[3px] focus-visible:ring-ring/50 disabled:cursor-not-allowed disabled:opacity-50 aria-invalid:border-destructive aria-invalid:ring-destructive/20 data-[state=checked]:border-primary data-[state=checked]:bg-primary data-[state=checked]:text-primary-foreground dark:bg-input/30 dark:aria-invalid:ring-destructive/40 dark:data-[state=checked]:bg-primary`

Structure: 16×16 px, 4 px radius, check icon `size-3.5` (14 px) in
`text-current` when checked; indeterminate shows a minus icon. Checked:
primary bg + primary-foreground check.

### 3.6 Select (dropdown trigger + content)

Trigger base: `flex h-9 w-full items-center justify-between whitespace-nowrap rounded-md border border-input bg-transparent px-3 py-2 text-sm shadow-xs outline-none placeholder:text-muted-foreground focus:border-ring focus:ring-[3px] focus:ring-ring/50 disabled:cursor-not-allowed disabled:opacity-50 [&>span]:truncate [&_svg]:shrink-0 dark:bg-input/30` + trailing `ChevronDown` (16 px, `text-muted-foreground`).

Content: `relative z-[var(--layer-popover,1000000)] min-w-[8rem] overflow-x-hidden overflow-y-auto overscroll-contain rounded-md border bg-popover text-popover-foreground shadow-md` + open/close animations (`animate-in fade-in-0 zoom-in-95` / `animate-out fade-out-0 zoom-out-95 fill-mode-forwards`) + side slide-ins (2 px).

Behavior: 36 px trigger; popover floats at `--layer-popover`; 8 px radius; `popover` bg + `shadow-md`; 4 px side offset translate on open.

### 3.7 Dialog (modal)

Overlay: `fixed inset-0 z-50 bg-black/50` + fade in/out (150 ms).

Content: `fixed top-[50%] left-[50%] z-50 grid max-h-[100dvh] w-full max-w-[calc(100%-2rem)] translate-x-[-50%] translate-y-[-50%] gap-4 overflow-y-auto rounded-lg border border-border/30 bg-background p-6 shadow-lg` + open `animate-in fade-in-0 zoom-in-95`, close `animate-out fade-out-0 zoom-out-95 fill-mode-forwards`; `sm:max-w-lg md:max-h-[100vh]`.

Header: title `text-lg leading-none font-semibold` (18 px); description
`text-sm text-muted-foreground` (14 px muted). Close button top-right:
`rounded-xs opacity-70 ring-offset-background transition-opacity hover:opacity-100 focus-visible:ring-2 focus-visible:ring-ring` with X (16 px).

Footer: `flex flex-col-reverse gap-2 sm:flex-row sm:justify-end` (Cancel
outline + primary action).

Behavior: centered, max 512 px (lg), 24 px padding, 8 px radius, 30% border,
shadow-lg, black/50 overlay; gap-4 between sections.

### 3.8 AlertDialog

Same chrome as Dialog; action button destructive variant for destructive
flows; Cancel + Action in footer.

### 3.9 Sheet (side panel)

Side variants (exact positions): right `inset-y-0 right-0 h-full w-3/4
border-l` … left/`top`/`bottom` mirror; content `bg-background shadow-lg
data-[state=open]:animate-in data-[state=closed]:animate-out` with
slide-in-from-* per side; overlay `bg-black/50`.

### 3.10 Tooltip

Content: `z-50 w-fit origin-(--bits-tooltip-content-transform-origin) animate-in rounded-md bg-primary px-3 py-1.5 text-xs text-primary-foreground fade-in-0 zoom-in-95` + side slide-ins 2 px + close animations. Arrow: `size-2.5 rotate-45 rounded-[2px] bg-primary`, positioned per side (`data-[side=top]:translate-x-1/2 data-[side=top]:translate-y-[calc(-50%_+_2px)]` etc.).

Behavior: 12 px text, primary bg, 6 px radius, 12×6 px padding; 300 ms delay (app-level).

### 3.11 Badge

Variants: **default** `inline-flex items-center justify-center gap-2 whitespace-nowrap rounded-md border px-2.5 py-0.5 text-xs font-semibold w-fit transition-[color,box-shadow] [&>svg]:pointer-events-none [&>svg]:size-3 shrink-0 bg-primary text-primary-foreground`; **secondary** `bg-secondary text-secondary-foreground`; **destructive**; **outline** `text-foreground`; plus `backdrop-blur-sm`. Used for model quantization/tags (Q8_0, Q4_K_M, vision, reasoning).

### 3.12 Card

`flex flex-col gap-6 rounded-xl bg-card py-6 text-card-foreground shadow-sm` + `border border-border/30 dark:border-border/20`. Header `flex flex-col gap-1.5`, title `leading-none font-semibold` (with `text-lg` variants), description `text-sm text-muted-foreground`.

Behavior: 12 px radius, `card` bg, shadow-sm, 24 px vertical padding, 24 px gap, 30% border. (Settings sections do NOT use cards — plain groups, §3.16.)

### 3.13 Separator / Skeleton

Separator: `shrink-0 bg-border` horizontal `h-px w-full` / vertical `w-px`.
Skeleton: `animate-pulse rounded-md bg-accent`.

### 3.14 DropdownMenu

Content: `z-50 max-h-… min-w-[8rem] overflow-x-hidden overflow-y-auto rounded-md border border-border bg-popover p-1.5 text-popover-foreground shadow-md outline-none dark:border-border/20` + open/close animations + side slide-ins 2 px.

Item: `relative flex cursor-pointer items-center gap-2 rounded-sm px-2 py-1.5 text-sm outline-hidden select-none data-highlighted:bg-accent data-highlighted:text-accent-foreground data-[disabled]:pointer-events-none data-[disabled]:opacity-50 data-[inset]:pl-8 data-[variant=destructive]:text-destructive data-[variant=destructive]:data-highlighted:bg-destructive/10 dark:data-[variant=destructive]:data-highlighted:bg-destructive/20 [&_svg]:size-4 [&_svg]:text-muted-foreground` — 6 px radius, 4 px radius items, accent highlight, 8 px x-padding / 6 px y-padding, 14 px text, `p-1.5` container.

### 3.15 Collapsible

No visual chrome; trigger = ghost button with chevron `h-4 w-4
text-muted-foreground transition-transform` (rotate 90° when open); content
`space-y-1`.

### 3.16 Settings fields (registry-driven)

- Label row: `<div class="flex items-center gap-2"><label class="flex items-center gap-1.5 text-sm font-medium">Label</label> (+ parameter-source indicator when the value drifts from the server default)</div>`
- Input (numbers/text): `w-full bg-muted/60 dark:bg-muted/75 border border-border/30 focus-within:border-border dark:border-border/20 dark:focus-within:border-border shadow-sm outline-none text-foreground rounded-md px-3 py-2 text-sm` + placeholder `Default: {serverValue}` when the server provides one; a `↺` reset button (`absolute top-1/2 right-2 h-5 w-5 … hover:bg-muted`) appears when the value differs from the server default
- Textarea: same chrome, `min-h-[10rem] w-full md:max-w-3xl`
- Help: `<p class="mt-1 text-xs text-muted-foreground">`
- Groups: `<div><h3 class="mb-6 text-base font-semibold">Title</h3><div class="space-y-8">fields</div></div>`

### 3.17 Sidebar (icon strip)

Aside: `fixed md:sticky top-2 left-2 md:left-0 md:ml-2 md:mt-2 pt-2 z-10 w-[calc(100dvw-1rem)] md:h-[calc(100dvh-1.125rem)] rounded-3xl md:rounded-2xl flex flex-col justify-between md:transition-[width,padding] duration-200 ease-out`.

- **Collapsed** (desktop): `md:w-12` — logo button (`md:h-9 md:w-9 h-10 w-10 rounded-full md:hover:bg-foreground/10!`, icon 16–18 px, tooltip "Expand navigation"; hovering shows PanelLeftOpen) + 4 icon actions (New chat, Search, MCP Servers, Settings — `md:h-9 md:w-9 h-10 w-10 rounded-full text-muted-foreground hover:bg-foreground/10 hover:text-foreground`, tooltips, shortcuts ⌘⇧O / ⌘K)
- **Expanded**: `md:w-72 md:bg-muted/60 md:backdrop-blur-xl border-border shadow-md is-expanded` — New Chat primary button (`w-full rounded-md bg-primary px-3 py-2 text-sm font-medium`), conversation list (`space-y-0.5`, rows `flex items-center gap-2 rounded-md px-3 py-2 text-sm hover:bg-accent`, active `bg-accent text-accent-foreground`), version footer (`text-[11px] opacity-50`)
- Mobile: full-width drawer (`w-[calc(100dvw-1rem)]`, `rounded-3xl`)

### 3.18 Chat screen

- Greeting (empty chat): `mb-2 text-2xl font-semibold tracking-tight md:text-3xl` + subtitle `text-muted-foreground md:text-lg`
- Composer: container `relative min-h-12 items-center rounded-4xl md:rounded-3xl py-2 pb-2.25 shadow-sm transition-all focus-within:shadow-md md:py-3!` (32 px mobile / 24 px desktop corner radius, shadow-sm → shadow-md on focus-within) — floating at `--chat-form-bottom-position`, `max-w-3xl mx-auto`
- Message bubbles: `.chat-message` block; assistant/user rows with `space-y-4`; per-message actions (copy, regenerate, delete) `opacity-0 group-hover:opacity-100` icon buttons (16 px, `text-muted-foreground hover:bg-accent rounded-md p-1.5`)
- Code blocks: max height `22rem` (`MAX_HEIGHT_CODE_BLOCK`), header row with language label + copy button, `bg-code-background text-code-foreground` for the block, `--font-mono`
- Context gauge / stats: `text-xs text-muted-foreground tabular-nums`

### 3.19 Settings page shell

- Wrapper: `mx-auto flex h-full w-full flex-col md:pl-8`
- Desktop nav: `sticky top-2 hidden w-64 flex-col self-start bg-background py-4 md:flex gap-6` — header row `flex items-center gap-2 py-2` (Settings icon `h-5 w-5 md:h-6 md:w-6` + `h1 class="text-xl font-semibold md:text-2xl"`), nav `space-y-1` with items `flex w-full items-center gap-3 rounded-lg px-3 py-2 text-left text-sm` + icon 16 px + `ml-2` span; active `bg-accent text-accent-foreground`, inactive `text-muted-foreground`, hover `hover:bg-accent`; footer `mt-auto pt-4 text-center opacity-50 text-[11px]` version line
- Content: `mx-auto max-w-3xl flex-1` → `space-y-6 p-4 md:p-6 md:pt-28` — section title row `mb-6 flex items-center gap-2 border-b border-border/30 pb-6` (icon 20 px + `h3 class="text-lg font-semibold"`), groups `space-y-6`, footer note `mt-8 border-t border-border/30 pt-6` + `text-xs text-muted-foreground` ("Settings are saved in browser's localStorage")
- Bottom action bar: `sticky bottom-0 mx-auto mt-4 flex w-full justify-between p-6` — Reset to default (outline button, `↺` icon 12 px) + Save settings (primary button)

---

## 4. Text & i18n

- Every user-visible string is a **dictionary key**; the 12-language dicts
  (`frontend/v2/lang/*.json`, embedded as `web/kernel/dicts.generated.js`)
  are the source of truth. `t('key')` falls back to the key itself.
- Component text maps to keys verbatim, e.g. `New Chat`, `Search`,
  `Settings`, `Save settings`, `Reset to default`, `Cancel`, `Default`,
  `Search presets...`, `No presets yet`, `Delete`, `Edit`, `Pin`.
- Labels with interpolation use `{token}` placeholders
  (e.g. `Allow use of {tool}?`, `Delete "{name}"?`).
- Icon-only buttons MUST carry `title`/`aria-label` with the t() string.

## 5. Implementation notes

1. **Web parity (current):** `web/kernel/tokens.css` already contains §2
   verbatim. The remaining work is aligning the vanilla primitives with §3:
   buttons/inputs/dialogs/tooltips in the vanilla `web/` tree should carry
   the exact class strings listed here (they were built from a slightly
   different shadcn generation).
2. **Native (future, web-wrapper-free):** §2 provides the complete palette
   per theme; §3 provides geometry (sizes, radii, shadows, spacing) and
   behavior (states, animations, focus rings). A GTK implementation maps
   tokens → CSS (GTK 4 supports CSS theming) and §3 components → GTK
   widgets (GtkButton with css classes per variant, GtkPopover, etc.).
   i18n stays dict-driven; z-layers and the chat layout vars in §1.5/§1.4
   translate directly.
3. **Verification:** pixel-diff against the v0.4.3 baseline screenshots
   (`parity-baseline/`) via `scripts/diff-parity.mjs`; acceptance ≤ 1% per
   view.
