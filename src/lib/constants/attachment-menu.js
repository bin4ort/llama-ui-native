import { MessageSquare, Zap, FolderOpen } from '@lucide/svelte';
import { FILE_TYPE_ICONS } from '$lib/constants/icons';
import { AttachmentAction, AttachmentItemEnabledWhen, AttachmentItemVisibleWhen, AttachmentMenuItemId } from '$lib/enums';
/**
 * File attachment menu items shown in both the desktop dropdown and mobile sheet.
 * The "Tools" submenu is handled separately by each component.
 */
export const ATTACHMENT_FILE_ITEMS = [
    {
        id: AttachmentMenuItemId.IMAGES,
        label: 'Images',
        icon: FILE_TYPE_ICONS.image,
        class: 'images-button',
        enabledWhen: AttachmentItemEnabledWhen.HAS_VISION_MODALITY,
        disabledTooltip: 'Image processing requires a vision model',
        action: AttachmentAction.FILE_UPLOAD
    },
    {
        id: AttachmentMenuItemId.AUDIO,
        label: 'Audio Files',
        icon: FILE_TYPE_ICONS.audio,
        class: 'audio-button',
        enabledWhen: AttachmentItemEnabledWhen.HAS_AUDIO_MODALITY,
        disabledTooltip: 'Audio files processing requires an audio model',
        action: AttachmentAction.FILE_UPLOAD
    },
    {
        id: AttachmentMenuItemId.VIDEO,
        label: 'Video Files',
        icon: FILE_TYPE_ICONS.video,
        class: 'video-button',
        enabledWhen: AttachmentItemEnabledWhen.HAS_VIDEO_MODALITY,
        disabledTooltip: 'Video files processing requires a video model',
        action: AttachmentAction.FILE_UPLOAD
    },
    {
        id: AttachmentMenuItemId.TEXT,
        label: 'Text Files',
        icon: FILE_TYPE_ICONS.text,
        enabledWhen: AttachmentItemEnabledWhen.ALWAYS,
        action: AttachmentAction.FILE_UPLOAD
    },
    {
        id: AttachmentMenuItemId.PDF,
        label: 'PDF Files',
        icon: FILE_TYPE_ICONS.pdf,
        enabledWhen: AttachmentItemEnabledWhen.ALWAYS,
        disabledTooltip: 'PDFs will be converted to text. Image-based PDFs may not work properly.',
        hasEnabledTooltip: true,
        action: AttachmentAction.FILE_UPLOAD
    }
];
export const ATTACHMENT_EXTRA_ITEMS = [];
export const ATTACHMENT_PROMPT_ITEMS = [
    {
        id: AttachmentMenuItemId.SYSTEM_MESSAGE,
        label: 'System Message',
        icon: MessageSquare,
        enabledWhen: AttachmentItemEnabledWhen.ALWAYS,
        hasEnabledTooltip: true,
        action: AttachmentAction.SYSTEM_PROMPT_CLICK
    },
    {
        id: AttachmentMenuItemId.MCP_PROMPT,
        label: 'MCP Prompt',
        icon: Zap,
        enabledWhen: AttachmentItemEnabledWhen.ALWAYS,
        action: AttachmentAction.MCP_PROMPT_CLICK,
        visibleWhen: AttachmentItemVisibleWhen.HAS_MCP_PROMPTS_SUPPORT
    }
];
export const ATTACHMENT_MCP_ITEMS = [
    {
        id: AttachmentMenuItemId.MCP_RESOURCES,
        label: 'MCP Resources',
        icon: FolderOpen,
        enabledWhen: AttachmentItemEnabledWhen.ALWAYS,
        action: AttachmentAction.MCP_RESOURCES_CLICK,
        visibleWhen: AttachmentItemVisibleWhen.HAS_MCP_RESOURCES_SUPPORT
    }
];
export const ATTACHMENT_TOOLTIP_TEXT = 'Add files, prompts, tools or MCP Servers';
