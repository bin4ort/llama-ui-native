---
name: ui-spec
description: Use when building, porting, or styling any UI element of Llama UI Native — chat messages, composer, settings, dialogs, sidebar, buttons, forms, MCP, models. The project has an exact per-component UI specification in docs/UI-SPEC.md and docs/UI-SPEC/ that must be followed class-for-class, not approximated. Trigger words: UI, GUI, style, visual, layout, component, look, appearance, design, parity.
---

# UI Specification (Llama UI Native)

Every user-visible element of this application has an exact, written
specification derived from the v0.4.3 interface. **Follow it class-for-class —
do not re-design, approximate, or "improve" the visuals.**

## Where the spec lives

| File | Contents |
|---|---|
| `docs/UI-SPEC.md` | Design tokens: radius scale (6/8/12/16/24 px), shadows (xs–lg), fonts, z-layers, chat layout vars; full color system (light/dark oklch tables + all 11 palette deltas); global state conventions (focus rings, disabled, dark variants) |
| `docs/UI-SPEC/00-index.md` | Complete component inventory (207 components + pages) |
| `docs/UI-SPEC/01-primitives.md` | The 26 shared UI primitives |
| `docs/UI-SPEC/02-shell-navigation.md` | Sidebar (icon strip + expanded), ActionIcon, SearchInput, server states |
| `docs/UI-SPEC/03-chat-messages.md` | Message tree, tool-call blocks, stats, action icons |
| `docs/UI-SPEC/04-chat-attachments.md` | Attachment list/preview |
| `docs/UI-SPEC/05-chat-form.md` | Composer, add menu, persona picker, context gauge |
| `docs/UI-SPEC/06-chat-screen.md` | Chat screen, greeting, banners |
| `docs/UI-SPEC/07-content.md` | Markdown, code blocks, mermaid |
| `docs/UI-SPEC/08-dialogs.md` | All 18 dialogs |
| `docs/UI-SPEC/09-settings.md` | Settings shell, fields, tables |
| `docs/UI-SPEC/10-mcp.md` | MCP pages/cards |
| `docs/UI-SPEC/11-models.md` | Model selectors |
| `docs/UI-SPEC/12-misc.md` | Logo, carousel, kbd, forms |
| `docs/UI-SPEC/13-pages.md` | Routes + native specifics |

## Rules

1. **Read the spec first.** Before writing or editing any UI code, open the
   relevant spec file and copy the exact class strings and metrics verbatim.
2. **Never invent styling.** If a class, size, or color is not in the spec,
   it does not exist. Do not add bespoke values.
3. **Use the token variables**, never raw colors: `bg-primary`,
   `text-muted-foreground`, `border-border/30`, `bg-muted/60`, etc. — they
   change per theme automatically (11 palettes). Raw hex/oklch only in
   `web/kernel/tokens.css`.
4. **State behavior is part of the spec**: focus rings
   (`focus-visible:ring-[3px] focus-visible:ring-ring/50`), disabled
   (`disabled:opacity-50`), hover alphas, open/close animations. Port them
   all.
5. **Icons** come from `web/settings/icons.js` (lucide paths, 16 px default
   via `h-4 w-4`). No new icon dependencies.
6. **i18n**: every user-visible string is a dictionary key through `t()`
   (12 languages, key-fallback). Never hardcode UI strings.
7. **Verification**: after UI changes, run `npm run build:web`,
   `cp -a dist-web/. frontend/v3/`, and — when the old-app baseline exists —
   `node scripts/capture-vanilla.mjs && node scripts/diff-parity.mjs
   tests/vanilla-current` (acceptance ≤ 1 % per view).
8. **Ownership**: Agent A owns `web/app/**` (shell, chat, chatbar); Agent B
   owns `web/settings/**` (settings, MCP, presets, dialogs, PWA). The kernel
   (`web/kernel/**`) is frozen — changes are contract changes logged in
   REFACTOR-PLAN.md §4.

## Short reference (the most-used values)

- Radius: `rounded-md` 6 px · `rounded-lg` 8 px · `rounded-xl` 12 px ·
  `rounded-2xl` 16 px · `rounded-3xl` 24 px · `rounded-4xl` 32 px.
- Button default: `h-9 px-4 py-2 rounded-md text-sm font-medium bg-primary
  text-primary-foreground shadow-sm hover:bg-primary/90`.
- Input: `bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm
  outline-none text-foreground rounded-md px-3 py-2 text-sm` (INPUT_CLASSES).
- Dialog: overlay `bg-black/50`, content `rounded-lg border border-border/30
  bg-background p-6 shadow-lg sm:max-w-lg`.
- Sidebar collapsed `md:w-12`, expanded `md:w-72 md:bg-muted/60
  md:backdrop-blur-xl border-border shadow-md`.
- Message bubbles: user `rounded-[1.125rem] bg-primary/5 px-3.75 py-1.5
  backdrop-blur-md`; system/persona `rounded-[1.125rem] bg-muted` (dashed
  border while editing).
- Composer: `rounded-4xl md:rounded-3xl py-2 pb-2.25 shadow-sm
  focus-within:shadow-md md:py-3!`.
- z-layers: sidebar 10, modal 50, popover `--layer-popover: 1000000`.
