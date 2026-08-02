/**
 * Utility functions for markdown processing in MarkdownContent component.
 */
/**
 * Generates a unique identifier for a HAST node based on its position.
 * Used for stable block identification during incremental rendering.
 * @param node - The HAST root content node
 * @param indexFallback - Fallback index if position is unavailable
 * @returns Unique string identifier for the node
 */
export function getHastNodeId(node, indexFallback) {
    const position = node.position;
    if (position?.start?.offset != null && position?.end?.offset != null) {
        return `hast-${position.start.offset}-${position.end.offset}`;
    }
    return `${node.type}-${indexFallback}`;
}
/**
 * Generates a hash for MDAST node based on its position.
 * Used for cache lookup during incremental rendering.
 */
export function getMdastNodeHash(node, index) {
    const n = node;
    if (n.position?.start?.offset != null && n.position?.end?.offset != null) {
        return `${n.type}-${n.position.start.offset}-${n.position.end.offset}`;
    }
    return `${n.type}-idx${index}`;
}
/**
 * Determines if the new content is an append (new content added to existing blocks).
 * This is used to optimize cache reuse during streaming updates.
 *
 * @param newContent - The new markdown content
 * @param previousContent - The previous markdown content to check against
 * @returns true if the content appears to be an append operation
 */
export function isAppendMode(newContent, previousContent) {
    return previousContent.length > 0 && newContent.startsWith(previousContent);
}
/**
 * Extracts code information from a button click target within a code block.
 * @param target - The clicked button element
 * @returns Object with rawCode and language, or null if extraction fails
 */
export function getCodeInfoFromTarget(target) {
    const wrapper = target.closest('.code-block-wrapper');
    if (!wrapper) {
        console.error('No wrapper found');
        return null;
    }
    const codeElement = wrapper.querySelector('code[data-code-id]');
    if (!codeElement) {
        console.error('No code element found in wrapper');
        return null;
    }
    const rawCode = codeElement.textContent ?? '';
    const languageLabel = wrapper.querySelector('.code-language');
    const language = languageLabel?.textContent?.trim() || 'text';
    return { rawCode, language };
}
