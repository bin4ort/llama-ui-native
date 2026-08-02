import { visit } from 'unist-util-visit';
import { AttachmentType, UrlProtocol } from '$lib/enums';
/**
 * Rehype plugin to resolve attachment image sources.
 * Converts attachment names (e.g., "mcp-attachment-xxx.png") to base64 data URLs.
 */
export function rehypeResolveAttachmentImages(options) {
    return (tree) => {
        visit(tree, 'element', (node) => {
            if (node.tagName === 'img' && node.properties?.src) {
                const src = String(node.properties.src);
                // Skip data URLs and external URLs
                if (src.startsWith(UrlProtocol.DATA) || src.startsWith(UrlProtocol.HTTP)) {
                    return;
                }
                // Find matching attachment
                const attachment = options.attachments?.find((a) => a.type === AttachmentType.IMAGE && a.name === src);
                // Replace with base64 URL if found
                if (attachment?.base64Url) {
                    node.properties.src = attachment.base64Url;
                }
            }
        });
    };
}
