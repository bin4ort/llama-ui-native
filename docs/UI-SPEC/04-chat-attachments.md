# 04 — Chat Attachments

## 4.1 ChatAttachments (composer attachments bar)

- Renders above the composer: selected files as chips/thumbnails with
  remove buttons; wrapper `flex flex-wrap items-center gap-2 px-3 py-2`.
- Entry button in the add menu: "Add files, prompts, tools or MCP Servers"
  (`text-muted-foreground` 14 px) — see §05.

## 4.2 List (attached in the composer)

- ChatAttachmentsList: `flex flex-wrap gap-2`.
- ChatAttachmentsListItem: chip `group flex items-center gap-2 rounded-lg
  border border-border/30 bg-muted/40 py-1 pl-1 pr-2 text-xs` — thumbnail
  24×24 `rounded-md overflow-hidden` + name `max-w-40 truncate` + size
  `text-muted-foreground` + remove `h-4 w-4 text-muted-foreground
  hover:text-destructive opacity-0 group-hover:opacity-100`.
- Variants: ThumbnailFile (file icon 16 px on `bg-muted`),
  ThumbnailImage (img `object-cover h-full w-full`),
  McpPrompt chip (`bg-purple-200/60` + template name),
  McpResource chip (`bg-blue-200/60` + resource name).

## 4.3 Preview (lightbox)

- ChatAttachmentsPreview: full-screen dialog (`Dialog.Content` chrome,
  `max-w-4xl`): header with file name + type + size
  (`flex items-center gap-2 text-xs text-muted-foreground`), main area
  centered, nav buttons (prev/next `ActionIcon h-8 w-8`),
  Close button top-right, footer action bar (Insert / Copy / Download /
  Delete buttons per type).
- Current-item variants:
  - Image: `max-h-[75vh] object-contain rounded-lg` + zoom controls
    (zoom in/out/fit, `flex items-center gap-1 rounded-lg bg-muted p-1`).
  - Audio: `<audio controls>` full width; play/pause + time display.
  - Video: `<video controls>` `max-h-[75vh]`.
  - PDF: pdf.js canvas in a scrollable frame `h-[70vh] w-full overflow-auto`.
  - Text: `<pre class="whitespace-pre-wrap text-sm p-4">`.
  - Unavailable: icon 32 px muted + "Preview not available" 14 px muted.
- PreviewFileInfo: file meta line `text-xs text-muted-foreground`.
- PreviewNavButtons: `absolute top-1/2 -translate-y-1/2` left/right
  circular buttons `h-9 w-9 rounded-full bg-background/80 shadow-md
  backdrop-blur hover:bg-background`.

## 4.4 DialogChatAttachmentsPreview

Same content in a dialog shell (used for viewing already-attached files);
grid `max-h-[90vh] max-w-5xl overflow-hidden`.
