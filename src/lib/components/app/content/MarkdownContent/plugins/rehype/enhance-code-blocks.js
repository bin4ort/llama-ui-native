/**
 * Rehype plugin to enhance code blocks with wrapper, header, and action buttons.
 *
 * Wraps <pre><code> elements with a container that includes:
 * - Language label
 * - Copy button
 * - Preview button (for HTML code blocks)
 *
 * This operates directly on the HAST tree for better performance,
 * avoiding the need to stringify and re-parse HTML.
 */
import { visit } from 'unist-util-visit';
import { CODE_BLOCK_SCROLL_CONTAINER_CLASS, CODE_BLOCK_WRAPPER_CLASS } from '$lib/constants';
import { createBlockHeader, createCopyButton, createPreviewButton, createWrapper, generateBlockId } from './code-block-utils';
function extractLanguage(codeElement) {
    const className = codeElement.properties?.className;
    if (!Array.isArray(className))
        return 'text';
    for (const cls of className) {
        if (typeof cls === 'string' && cls.startsWith('language-')) {
            return cls.replace('language-', '');
        }
    }
    return 'text';
}
/**
 * Rehype plugin to enhance code blocks with wrapper, header, and action buttons.
 * This plugin wraps <pre><code> elements with a container that includes:
 * - Language label
 * - Copy button
 * - Preview button (for HTML code blocks)
 */
export const rehypeEnhanceCodeBlocks = () => {
    return (tree) => {
        visit(tree, 'element', (node, index, parent) => {
            if (node.tagName !== 'pre' || !parent || index === undefined)
                return;
            const codeElement = node.children.find((child) => child.type === 'element' && child.tagName === 'code');
            if (!codeElement)
                return;
            const language = extractLanguage(codeElement);
            const codeId = generateBlockId('code', 'idxCodeBlock');
            codeElement.properties = {
                ...codeElement.properties,
                'data-code-id': codeId
            };
            const actions = [createCopyButton(codeId, 'data-code-id', 'Copy code')];
            if (language.toLowerCase() === 'html') {
                actions.push(createPreviewButton(codeId, 'data-code-id', 'Preview code'));
            }
            const header = createBlockHeader(language, codeId, 'data-code-id', actions);
            const wrapper = createWrapper(header, node, CODE_BLOCK_WRAPPER_CLASS, CODE_BLOCK_SCROLL_CONTAINER_CLASS);
            // Replace pre with wrapper in parent
            parent.children[index] = wrapper;
        });
    };
};
