export var ChatMessageStatsView;
(function (ChatMessageStatsView) {
    ChatMessageStatsView["GENERATION"] = "generation";
    ChatMessageStatsView["READING"] = "reading";
    ChatMessageStatsView["TOOLS"] = "tools";
    ChatMessageStatsView["SUMMARY"] = "summary";
})(ChatMessageStatsView || (ChatMessageStatsView = {}));
export var ChatMessageStatisticsMode;
(function (ChatMessageStatisticsMode) {
    ChatMessageStatisticsMode["SWITCHABLE"] = "switchable";
    ChatMessageStatisticsMode["READING"] = "reading";
    ChatMessageStatisticsMode["GENERATION"] = "generation";
})(ChatMessageStatisticsMode || (ChatMessageStatisticsMode = {}));
/**
 * Connection state of a streamed completion, drives the resume status indicator.
 */
export var StreamConnectionState;
(function (StreamConnectionState) {
    StreamConnectionState["STREAMING"] = "streaming";
    StreamConnectionState["RESUMING"] = "resuming";
    StreamConnectionState["LOST"] = "lost";
})(StreamConnectionState || (StreamConnectionState = {}));
/**
 * Reasoning format options for API requests.
 */
export var ReasoningFormat;
(function (ReasoningFormat) {
    ReasoningFormat["NONE"] = "none";
    ReasoningFormat["AUTO"] = "auto";
})(ReasoningFormat || (ReasoningFormat = {}));
/**
 * Message roles for chat messages.
 */
export var MessageRole;
(function (MessageRole) {
    MessageRole["USER"] = "user";
    MessageRole["ASSISTANT"] = "assistant";
    MessageRole["SYSTEM"] = "system";
    MessageRole["TOOL"] = "tool";
})(MessageRole || (MessageRole = {}));
/**
 * Message types for different content kinds.
 */
export var MessageType;
(function (MessageType) {
    MessageType["ROOT"] = "root";
    MessageType["TEXT"] = "text";
    MessageType["THINK"] = "think";
    MessageType["SYSTEM"] = "system";
})(MessageType || (MessageType = {}));
/**
 * Content part types for API chat message content.
 */
export var ContentPartType;
(function (ContentPartType) {
    ContentPartType["TEXT"] = "text";
    ContentPartType["IMAGE_URL"] = "image_url";
    ContentPartType["INPUT_AUDIO"] = "input_audio";
    ContentPartType["INPUT_VIDEO"] = "input_video";
})(ContentPartType || (ContentPartType = {}));
/**
 * Error dialog types for displaying server/timeout errors.
 */
export var ErrorDialogType;
(function (ErrorDialogType) {
    ErrorDialogType["TIMEOUT"] = "timeout";
    ErrorDialogType["SERVER"] = "server";
})(ErrorDialogType || (ErrorDialogType = {}));
export var ConversationSelectionMode;
(function (ConversationSelectionMode) {
    ConversationSelectionMode["EXPORT"] = "export";
    ConversationSelectionMode["IMPORT"] = "import";
})(ConversationSelectionMode || (ConversationSelectionMode = {}));
/**
 * PDF view mode options for previewing PDF attachments.
 */
export var PdfViewMode;
(function (PdfViewMode) {
    PdfViewMode["TEXT"] = "text";
    PdfViewMode["PAGES"] = "pages";
})(PdfViewMode || (PdfViewMode = {}));
