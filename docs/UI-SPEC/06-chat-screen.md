# 06 — Chat Screen

## 6.1 ChatScreen

- Main area: `chat-screen flex grow flex-col min-h-[calc(100dvh-1rem)]
  md:min-h-full px-4 md:py-0 pt-12 pb-48 md:pb-4` — `pb-48` reserves the
  floating composer space; composer positioned via
  `--chat-form-bottom-position` (1rem desktop / 0.5rem mobile).
- Renders ChatMessages (max-w-3xl/48rem) or, when empty (`showCenteredEmpty`),
  the ChatScreenGreeting centered + `md:bottom-[calc(50dvh-7rem)]` composer.
- Server loading → full-screen ServerLoadingSplash replaces the screen.

## 6.2 ChatScreenGreeting

- `flex flex-col items-center justify-center gap-2 text-center`:
  title `mb-2 text-2xl font-semibold tracking-tight md:text-3xl`
  ("What can I help you with?"), subtitle
  `text-muted-foreground md:text-lg` ("Start a conversation with your local
  AI — ask anything, upload files, or attach tools").
- Suggestion chips row (2×2 grid on desktop, horizontal scroll on mobile):
  pill `inline-flex items-center gap-1.5 rounded-full border bg-muted px-3
  py-1.5 text-xs hover:bg-muted/80` (Sparkles icon 12 px + suggestion text).

## 6.3 ChatScreenActionScrollDown

- Floating "scroll to bottom" pill when the user scrolled up:
  `absolute bottom-24 left-1/2 -translate-x-1/2 z-10 inline-flex items-center
  gap-1.5 rounded-full border bg-background/80 px-3 py-1.5 text-xs shadow-md
  backdrop-blur hover:bg-background` — ChevronDown 14 px + "New messages".

## 6.4 ChatScreenDialogsAndAlerts

- Mount point for: error dialog (ChatError), rename dialog, delete
  confirmation, empty-file alert, model-not-available dialog, toasts.

## 6.5 ChatScreenDragOverlay

- While dragging files: full-screen overlay
  `fixed inset-0 z-50 flex items-center justify-center bg-background/80
  backdrop-blur` with dashed drop target card (Plus icon 32 px, "Drop files
  to attach").

## 6.6 ChatScreenServerError

- Banner above the composer when the backend is unreachable:
  `flex items-center gap-2 rounded-lg border border-destructive/30
  bg-destructive/10 px-3 py-2 text-xs text-destructive` — "Server is not
  running or unreachable" + Retry (link-style button).

## 6.7 ChatScreenStreamResumeStatus

- During stream discovery/resume:
  `flex items-center gap-2 rounded-full border bg-muted/60 px-3 py-1 text-xs
  text-muted-foreground` — spinner 12 px + "Reconnecting to the active
  stream…" / "Streaming resumed".

## 6.8 ChatScreenSystemPromptHint

- Hint under an empty conversation (persona row exists): 12 px muted text
  "This conversation uses a system prompt — edit it from the message above."
