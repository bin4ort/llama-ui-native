/**
 * OpenAI-compatible tool call type.
 */
export var ToolCallType;
(function (ToolCallType) {
    ToolCallType["FUNCTION"] = "function";
})(ToolCallType || (ToolCallType = {}));
/**
 * Types of sections in agentic content display.
 */
export var AgenticSectionType;
(function (AgenticSectionType) {
    AgenticSectionType["TEXT"] = "text";
    AgenticSectionType["TOOL_CALL"] = "tool_call";
    AgenticSectionType["TOOL_CALL_PENDING"] = "tool_call_pending";
    AgenticSectionType["TOOL_CALL_STREAMING"] = "tool_call_streaming";
    AgenticSectionType["REASONING"] = "reasoning";
    AgenticSectionType["REASONING_PENDING"] = "reasoning_pending";
})(AgenticSectionType || (AgenticSectionType = {}));
/**
 * How a Continue click on an assistant message resumes generation.
 */
export var ContinueIntentKind;
(function (ContinueIntentKind) {
    ContinueIntentKind["APPEND_TEXT"] = "append_text";
    ContinueIntentKind["RERUN_TURN"] = "rerun_turn";
    ContinueIntentKind["NEXT_TURN"] = "next_turn";
})(ContinueIntentKind || (ContinueIntentKind = {}));
/**
 * Renderer tier for a tool-result blob shown in the default tool-call block.
 */
export var ToolResultKind;
(function (ToolResultKind) {
    ToolResultKind["JSON"] = "json";
    ToolResultKind["MARKDOWN"] = "markdown";
    ToolResultKind["TEXT"] = "text";
})(ToolResultKind || (ToolResultKind = {}));
/**
 * Line classification for the unified-diff renderer of `edit_file` results.
 */
export var DiffLineKind;
(function (DiffLineKind) {
    DiffLineKind["CONTEXT"] = "context";
    DiffLineKind["ADD"] = "add";
    DiffLineKind["REMOVE"] = "remove";
})(DiffLineKind || (DiffLineKind = {}));
