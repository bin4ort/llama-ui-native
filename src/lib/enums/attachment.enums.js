/**
 * Attachment type enum for database message extras
 */
export var AttachmentType;
(function (AttachmentType) {
    AttachmentType["AUDIO"] = "AUDIO";
    AttachmentType["IMAGE"] = "IMAGE";
    AttachmentType["VIDEO"] = "VIDEO";
    AttachmentType["MCP_PROMPT"] = "MCP_PROMPT";
    AttachmentType["MCP_RESOURCE"] = "MCP_RESOURCE";
    AttachmentType["PDF"] = "PDF";
    AttachmentType["TEXT"] = "TEXT";
    AttachmentType["LEGACY_CONTEXT"] = "context"; // Legacy attachment type for backward compatibility
})(AttachmentType || (AttachmentType = {}));
/**
 * Unique identifiers for attachment menu items in the chat form action dropdowns.
 * Used to select which file upload or attachment action is triggered.
 */
export var AttachmentMenuItemId;
(function (AttachmentMenuItemId) {
    AttachmentMenuItemId["IMAGES"] = "images";
    AttachmentMenuItemId["AUDIO"] = "audio";
    AttachmentMenuItemId["VIDEO"] = "video";
    AttachmentMenuItemId["TEXT"] = "text";
    AttachmentMenuItemId["PDF"] = "pdf";
    AttachmentMenuItemId["SYSTEM_MESSAGE"] = "system-message";
    AttachmentMenuItemId["MCP_PROMPT"] = "mcp-prompt";
    AttachmentMenuItemId["MCP_RESOURCES"] = "mcp-resources";
})(AttachmentMenuItemId || (AttachmentMenuItemId = {}));
/**
 * Defines when an attachment menu item should be enabled.
 */
export var AttachmentItemEnabledWhen;
(function (AttachmentItemEnabledWhen) {
    AttachmentItemEnabledWhen["ALWAYS"] = "always";
    AttachmentItemEnabledWhen["HAS_VISION_MODALITY"] = "hasVisionModality";
    AttachmentItemEnabledWhen["HAS_AUDIO_MODALITY"] = "hasAudioModality";
    AttachmentItemEnabledWhen["HAS_VIDEO_MODALITY"] = "hasVideoModality";
})(AttachmentItemEnabledWhen || (AttachmentItemEnabledWhen = {}));
/**
 * Defines the callback action triggered when an attachment menu item is clicked.
 */
export var AttachmentAction;
(function (AttachmentAction) {
    AttachmentAction["FILE_UPLOAD"] = "onFileUpload";
    AttachmentAction["SYSTEM_PROMPT_CLICK"] = "onSystemPromptClick";
    AttachmentAction["MCP_PROMPT_CLICK"] = "onMcpPromptClick";
    AttachmentAction["MCP_RESOURCES_CLICK"] = "onMcpResourcesClick";
})(AttachmentAction || (AttachmentAction = {}));
/**
 * Visibility conditions for attachment menu items.
 */
export var AttachmentItemVisibleWhen;
(function (AttachmentItemVisibleWhen) {
    AttachmentItemVisibleWhen["HAS_MCP_PROMPTS_SUPPORT"] = "hasMcpPromptsSupport";
    AttachmentItemVisibleWhen["HAS_MCP_RESOURCES_SUPPORT"] = "hasMcpResourcesSupport";
})(AttachmentItemVisibleWhen || (AttachmentItemVisibleWhen = {}));
