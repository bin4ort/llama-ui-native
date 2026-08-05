# 07 — Content Rendering (markdown, code, diagrams)

## 7.1 MarkdownContent

- Root: `markdown-content` (typography-prose container; Tailwind
  `@tailwindcss/typography` plugin, `prose`-style rules with token colors).
- Pipeline (in order): LaTeX protection (`$…$` / `$$…$$` ↔ § placeholders)
  → marked/GFM → DOMPurify sanitization → LaTeX re-render (katex,
  `katex-display` blocks, lazy CSS) → code blocks → mermaid (`<pre
  class="language-mermaid">` → inline SVG) → inline SVG blocks (sanitized
  with `dompurify`, `svg` + `svgFilters` profiles, FORBID_TAGS
  `foreignObject`, `script`) → image handling (paste/data URLs).
- Tables: rendered with `border-border` grid lines, header `bg-muted/50`,
  `text-sm`; inside messages `overflow-x-auto`.
- Lists: `list-disc`/`list-decimal` with `pl-6`; `space-y-1`.
- Headings: h1 24 px / h2 20 px / h3 16 px semibold, `mt-4 mb-2`.
- Links: `text-primary underline underline-offset-4 hover:underline`.
- Blockquote: `border-l-2 border-border pl-4 text-muted-foreground`.
- Inline code: `rounded bg-muted px-1.5 py-0.5 font-mono text-[0.875em]`.
- Images: `max-h-96 rounded-lg object-contain` with click-to-preview.

## 7.2 SyntaxHighlightedCode

- Wrapper: `code-preview-wrapper min-w-0 max-w-full overflow-auto rounded-xl
  border shadow-[0_1px_2px_0_rgb(0_0_0_/_0.05)]` (1 px border, subtle
  shadow); highlight.js theme CSS (github-dark/github light) injected
  per theme; `pre.hljs text-sm leading-relaxed p-4 m-0`.
- Max height `22rem` (`MAX_HEIGHT_CODE_BLOCK`) with internal scroll;
  full-height mode when `fullHeightCodeBlocks`.
- CodeBlockActions (misc): header row above — copy (ActionIconCopyToClipboard),
  preview (Maximize icon), language label `font-mono text-[10px]
  text-muted-foreground/70 uppercase tracking-wide`.

## 7.3 CollapsibleContentBlock

- Used for reasoning blocks: header button
  `flex w-full cursor-pointer items-start justify-between gap-2` — chevron
  `h-4 w-4 text-muted-foreground transition-transform` (rotate-90 open) +
  label 12 px muted; body `min-w-0 border-l border-muted-foreground/20
  pl-4 pb-2 my-2` + `pl-1.5 grid min-w-0`.

## 7.4 CollapsibleTerminalBlock

- Terminal-style collapsible (shell commands): header with Terminal icon +
  label `text-xs italic text-muted-foreground/70`; body `p-3 pt-1`
  `font-mono text-[11px] leading-relaxed whitespace-pre-wrap` on
  `bg-black/80 text-green-400` (dark) / `bg-muted text-foreground` (light).

## 7.5 MermaidPreview

- Full diagram view: `mermaid-preview relative flex items-center justify-center
  overflow-hidden bg-muted/20` — pan/zoom canvas
  `mermaid-preview-diagram transform-origin-center inline-block min-h-fit
  min-w-fit will-change-transform`; drag-to-pan, wheel-zoom (0.25–3×).
- Controls (MermaidPreviewControls): bottom-center bar
  `absolute bottom-8 flex shrink-0 items-center justify-center p-3` →
  inner `flex items-center gap-1 rounded-lg bg-muted p-1` with buttons
  `flex h-8 w-8 items-center justify-center rounded-md hover:bg-accent`
  (zoom out/in, reset, copy SVG, download SVG, close). Toggle buttons
  `mermaid-preview-toggle` top-right (View / Source).

## 7.6 Code preview dialog

- DialogCodePreview: full-screen `code-preview-overlay` (black) →
  `code-preview-content` (iframe, white bg) + close button
  `absolute top-4 right-4 border-none bg-transparent text-white opacity-70
  mix-blend-difference transition-opacity hover:opacity-100` — used for
  rendered HTML preview.
