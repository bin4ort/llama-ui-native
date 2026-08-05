# 08 — Application Dialogs

All dialogs use the Dialog primitive chrome (§01): overlay `bg-black/50`,
content `rounded-lg border border-border/30 bg-background p-6 shadow-lg
sm:max-w-lg`, title 18 px semibold, description 14 px muted, close button
top-right. Notes below are per-dialog specifics.

| Dialog | Purpose | Specifics |
|---|---|---|
| **ChatError** | Server/stream errors | Header `flex items-center gap-2` + AlertTriangle 20 px `text-destructive` + title; body `mt-2 space-y-1 text-xs opacity-80` error details (code + message); Retry button. |
| **ChatAttachmentsPreview** | View attached files | `max-h-[90vh] max-w-5xl overflow-hidden`; header meta + preview area + nav buttons + thumbnail strip `sticky bottom-0 z-10 mt-4` (`h-12 w-12 object-cover` thumbs). |
| **CodePreview** | Rendered HTML in iframe | Full-screen overlay, white iframe, mix-blend-difference close. |
| **Confirmation** | Generic confirm | Icon 20 px (variant: default/amber/red) + message; footer Cancel (outline) + Confirm (default/destructive). |
| **ConversationRename** | Rename a conversation | Input `space-y-2 pt-2 pb-4` + label; footer Cancel/Rename. |
| **ConversationSelection** | Batch-select conversations | `z-1000000` overlay (above normal modal layer); content `z-1000001 max-w-2xl`; marquee-selectable list + selection bar. |
| **EmptyFileAlert** | Empty files dropped | Warning icon 20 px `text-destructive`; message; file names `list-inside list-disc space-y-1 text-muted-foreground`. |
| **ExportSettings** | Export/import settings | Textarea with JSON `font-mono text-xs min-h-64 w-full`; amber warning `text-amber-500` + confirm checkbox; Copy / Download / Import buttons. |
| **FileUploadError** | Upload failures | `max-w-md`; list of failures `!max-h-[50vh] min-h-0 flex-1 space-y-4 overflow-y-auto`; per-file `text-sm font-medium text-destructive` + reason. |
| **McpResourcePreview** | MCP resource content | `grid max-h-[90vh] max-w-5xl overflow-hidden` — header (server avatar + name + type chip), content `text-xs` with copy button. |
| **McpResourcesBrowser** | Browse MCP resources | `max-h-[80vh] !max-w-4xl overflow-hidden p-0`; header `border-b border-border/30 px-6 py-4` (title + close); body `flex h-[500px] min-w-0` — server list left, resource detail right. |
| **McpServerAddNew** | Add MCP server | `sm:max-w-2xl`; form fields (name, URL, transport select), advanced collapsible (headers, auth), test button, error `text-destructive`; footer Cancel/Add. |
| **MermaidPreview** | Full mermaid diagram | `z-999999 grid max-h-full max-w-full! grid-rows-[1fr_auto] overflow-hidden p-0 md:h-[90vh] md:max-w-[90vw]!` — diagram canvas + controls. |
| **ModelInformation** | Model details | `@container z-9999 !max-h-[80dvh] !max-w-[60rem] max-w-full`; header (model name + badges), body: id, quantization, context, tags, file info `w-[10rem]` label/value rows; footer (select model / close). |
| **ModelNotAvailable** | Requested model missing | `max-w-lg`; amber header icon 20 px `text-amber-500`; warning panel `rounded-lg border border-amber-500/40 bg-amber-500/10 px-4 py-3 text-sm` + `font-medium text-amber-600 dark:text-amber-400`; available models list; Close. |
| **PresetPicker** | Choose a persona preset | `z-9999 !max-h-[85dvh] !max-w-2xl overflow-x-hidden overflow-y-auto`; search input; list `h-96 pr-3 space-y-1` with rows `flex w-full items-center justify-between gap-2 rounded-md px-3 py-2 text-left text-sm transition-colors hover:bg-accent` (active `bg-accent`); footer: Default (outline) + "Create with wizard…" (primary). |
| **PresetWizard** | Generate a persona | Same chrome `!max-w-2xl`; request textarea; Generate button (Sparkles 16 px, spinner while generating `mr-2 h-4 w-4 animate-spin`); draft panel (name, description, content `font-mono text-xs rows-10`); footer Cancel / Save preset. |
| **VerificationDialog** | Tool permission prompt | `z-9999`; header shield/sparkles 20 px `text-amber-400`; description `whitespace-pre-wrap text-sm`; extra args summary (mono, muted); footer Deny (outline) + Allow once (primary) + "Always allow" (secondary, staged). |
