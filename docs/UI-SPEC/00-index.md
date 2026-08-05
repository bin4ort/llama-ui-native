# UI-SPEC — Complete Component Inventory

Full per-component specification of the Llama UI Native interface, derived
from the v0.4.3 Svelte sources (git history `810c952~1`). Every element and
component is covered. Files:

| File | Covers |
|---|---|
| [../UI-SPEC.md](../UI-SPEC.md) | Design tokens (radii, shadows, fonts, z-layers), full color system (light/dark + 11 palettes), global state conventions |
| [01-primitives.md](01-primitives.md) | The 26 shared UI primitives (button, input, dialog, …) |
| [02-shell-navigation.md](02-shell-navigation.md) | SidebarNavigation (7), navigation (2), misc chrome (6), badges (2), actions (2), server (3), forms (3) |
| [03-chat-messages.md](03-chat-messages.md) | ChatMessages tree: message, system, user, assistant, reasoning, tool-call (12), stats, action cards/icons, MCP prompt/resource messages |
| [04-chat-attachments.md](04-chat-attachments.md) | Attachments list, list items (thumbnail variants), preview + current-item variants, file info, nav buttons |
| [05-chat-form.md](05-chat-form.md) | Composer, actions (add, preset, models, tools…), pickers (system prompt, MCP prompts/resources), context gauge |
| [06-chat-screen.md](06-chat-screen.md) | ChatScreen, greeting, scroll helpers, server error, stream resume, dialogs-and-alerts |
| [07-content.md](07-content.md) | Markdown rendering, syntax-highlighted code, collapsible blocks, mermaid preview + controls |
| [08-dialogs.md](08-dialogs.md) | All 18 application dialogs |
| [09-settings.md](09-settings.md) | Settings page shell, desktop sidebar, mobile header, footer, fields, groups |
| [10-mcp.md](10-mcp.md) | MCP servers page, server cards, resources browser, logos/badges, connection logs |
| [11-models.md](11-models.md) | Model badges, ids, selector dropdown/sheet/list |
| [12-misc.md](12-misc.md) | Logo, truncated text, keyboard hints, carousel, conversation selection, splashes |
| [13-pages.md](13-pages.md) | Routes: chat page, settings page, search page, MCP page, error page |

## Conventions used in every file

- **Structure**: the DOM hierarchy (tags + data-slot attributes).
- **Classes**: the exact class strings; `INPUT_CLASSES` / `BOX_BORDER` /
  `PANEL_CLASSES` refer to the shared constants (§01.0).
- **States**: hover / focus-visible / disabled / checked / open-close /
  editing / streaming, with the exact class deltas.
- **Text**: user-visible strings are dictionary keys (§4 of ../UI-SPEC.md);
  keys are written verbatim.
- **Theme behavior**: any `dark:` / palette-specific variance.
- **Metrics**: computed values (px) derived from the token scale.

## Component inventory (complete)

**UI primitives (26)** — alert, alert-dialog, badge, button, button-group,
card, checkbox, collapsible, dialog, dropdown-menu, empty, hover-card, input,
label, popover, radio-group, scroll-area, select, separator, sheet, skeleton,
switch, table, textarea, tooltip, utils(cn).

**Shell & navigation (15)** — SidebarNavigation, SidebarNavigationActions,
SidebarNavigationConversationItem, SidebarNavigationConversationList,
SidebarNavigationSearch, SidebarNavigationSearchResults,
SidebarNavigationSelectionBar, ActionIcon, ActionIconCopyToClipboard, Logo,
TruncatedText, HorizontalScrollCarousel, KeyboardShortcutInfo,
ConversationSelection, BadgeInfo, BadgesModality.

**Chat messages (33)** — ChatMessages, ChatMessage, ChatMessageSystem,
ChatMessageUser, ChatMessageAssistant, ChatMessageAssistantReasoningBlock,
ChatMessageAssistantThoughtInProgress, ChatMessageEditForm,
ChatMessageMcpPrompt, ChatMessageMcpResource, ChatMessageActionIcons,
ChatMessageActionCard (+4 variants), ChatMessageStatistics (+2),
ChatMessageToolCall (+12: parsers/renderers for built-in + MCP tools).

**Attachments (14)** — ChatAttachments, ChatAttachmentsList,
ChatAttachmentsListItem, …ListItemMcpPrompt, …ListItemMcpResource,
…ListItemThumbnailFile, …ListItemThumbnailImage, ChatAttachmentsPreview,
PreviewCurrentItem (+6 type variants), PreviewFileInfo, PreviewNavButtons.

**Chat form (23)** — ChatForm, ChatFormActions, ChatFormActionAdd (+7),
ChatFormActionPreset, ChatFormActionModels, ChatFormContextGauge (+6),
ChatFormPickers, ChatFormPicker (+2), ChatFormPickerMcpPrompts,
ChatFormPickerMcpResources.

**Chat screen (8)** — ChatScreen, ChatScreenActionScrollDown,
ChatScreenDialogsAndAlerts, ChatScreenDragOverlay, ChatScreenGreeting,
ChatScreenServerError, ChatScreenStreamResumeStatus,
ChatScreenSystemPromptHint.

**Content (7)** — MarkdownContent, SyntaxHighlightedCode, CollapsibleContentBlock,
CollapsibleTerminalBlock, MermaidPreview, MermaidPreviewControls, CodeBlockActions.

**Dialogs (18)** — ChatAttachmentsPreview, ChatError, CodePreview,
Confirmation, ConversationRename, ConversationSelection, EmptyFileAlert,
ExportSettings, FileUploadError, McpResourcePreview, McpResourcesBrowser,
McpServerAddNew, MermaidPreview, ModelInformation, ModelNotAvailable,
PresetPicker, PresetWizard, VerificationDialog.

**Settings (13)** — SettingsChat, SettingsChatDesktopSidebar,
SettingsChatMobileHeader, SettingsFooter, SettingsGroup,
SettingsChatSystemPromptPresets, SettingsChatFields,
SettingsChatParameterSourceIndicator, SettingsChatSeparator,
SettingsChatSystemPromptEditor, SettingsChatToolTable,
SettingsChatBuiltinToolTable, SettingsChatLinkButton.

**MCP (11)** — McpServers page, McpServerCard, McpServerCardSkeleton,
McpCapabilitiesBadges, McpActiveServersAvatars, McpConnectionLogs, McpLogo,
McpResourcePreview, McpResourceTemplateForm, McpResourcesBrowser (+3).

**Models (7)** — ModelBadge, ModelId, ModelLoadHighlight,
ModelsSelectorDropdown, ModelsSelectorList, ModelsSelectorOption,
ModelsSelectorSheet.

**Server/forms/misc (12)** — ServerErrorSplash, ServerLoadingSplash,
ServerStatus, InputWithSuggestions, KeyValuePairs, SearchInput,
Form… (composer forms).

**Pages (routes)** — chat `(chat)/+page`, `chat/[id]/+page`, `search/+page`,
`settings/[[section]]/+page`, `mcp-servers/+page`, `+error`, `+layout`.
