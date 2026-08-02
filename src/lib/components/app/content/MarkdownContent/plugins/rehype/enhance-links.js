/**
 * Rehype plugin to enhance links with security attributes.
 *
 * Adds target="_blank" and rel="noopener noreferrer" to all anchor elements,
 * ensuring external links open in new tabs safely.
 */
import { visit } from 'unist-util-visit';
/**
 * Rehype plugin that adds security attributes to all links.
 * This plugin ensures external links open in new tabs safely by adding:
 * - target="_blank"
 * - rel="noopener noreferrer"
 */
export const rehypeEnhanceLinks = () => {
    return (tree) => {
        visit(tree, 'element', (node) => {
            if (node.tagName !== 'a')
                return;
            const props = node.properties ?? {};
            // Only modify if href exists
            if (!props.href)
                return;
            props.target = '_blank';
            props.rel = 'noopener noreferrer';
            node.properties = props;
        });
    };
};
