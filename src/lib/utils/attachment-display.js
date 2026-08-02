import { AttachmentType, FileTypeCategory, SpecialFileType } from '$lib/enums';
import { getFileTypeCategory, getFileTypeCategoryByExtension, isImageFile } from '$lib/utils';
/**
 * Check if a display item represents an MCP prompt
 * (either from attachment type or uploaded file with mcpPrompt metadata)
 */
export function isMcpPrompt(item) {
    if (item.attachment?.type === AttachmentType.MCP_PROMPT) {
        return true;
    }
    if (item.uploadedFile?.type === SpecialFileType.MCP_PROMPT && item.uploadedFile.mcpPrompt) {
        return true;
    }
    return false;
}
/**
 * Check if a display item represents an MCP resource
 */
export function isMcpResource(item) {
    return item.attachment?.type === AttachmentType.MCP_RESOURCE;
}
/**
 * Gets the file type category from an uploaded file, checking both MIME type and extension
 */
function getUploadedFileCategory(file) {
    const categoryByMime = getFileTypeCategory(file.type);
    if (categoryByMime) {
        return categoryByMime;
    }
    return getFileTypeCategoryByExtension(file.name);
}
/**
 * Creates a unified list of display items from uploaded files and stored attachments.
 * Items are returned in reverse order (newest first).
 */
export function getAttachmentDisplayItems(options) {
    const { uploadedFiles = [], attachments = [] } = options;
    const items = [];
    // Add uploaded files (ChatForm)
    for (const file of uploadedFiles) {
        items.push({
            id: file.id,
            name: file.name,
            size: file.size,
            preview: file.preview,
            isImage: getUploadedFileCategory(file) === FileTypeCategory.IMAGE,
            isLoading: file.isLoading,
            loadError: file.loadError,
            uploadedFile: file,
            textContent: file.textContent
        });
    }
    // Add stored attachments (ChatMessage)
    for (const [index, attachment] of attachments.entries()) {
        const isImage = isImageFile(attachment);
        items.push({
            id: `attachment-${index}`,
            name: attachment.name,
            size: 'size' in attachment ? attachment.size : undefined,
            preview: isImage && 'base64Url' in attachment ? attachment.base64Url : undefined,
            isImage,
            attachment,
            attachmentIndex: index,
            textContent: 'content' in attachment ? attachment.content : undefined
        });
    }
    return items.reverse();
}
