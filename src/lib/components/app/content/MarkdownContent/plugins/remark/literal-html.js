import { visit } from 'unist-util-visit';
import { LINE_BREAK, NBSP, PHRASE_PARENTS, TAB_AS_SPACES } from '$lib/constants';
/**
 * remark plugin that rewrites raw HTML nodes into plain-text equivalents.
 *
 * remark parses inline HTML into `html` nodes even when we do not want to render
 * them. We turn each of those nodes into regular text (plus `<br>` break markers)
 * so the downstream rehype pipeline escapes the characters instead of executing
 * them. Leading spaces and tab characters are converted to non‑breaking spaces to
 * keep indentation identical to the original author input.
 */
function preserveIndent(line) {
    let index = 0;
    let output = '';
    while (index < line.length) {
        const char = line[index];
        if (char === ' ') {
            output += NBSP;
            index += 1;
            continue;
        }
        if (char === '\t') {
            output += TAB_AS_SPACES;
            index += 1;
            continue;
        }
        break;
    }
    return output + line.slice(index);
}
function createLiteralChildren(value) {
    const lines = value.split(LINE_BREAK);
    const nodes = [];
    for (const [lineIndex, rawLine] of lines.entries()) {
        if (lineIndex > 0) {
            nodes.push({ type: 'break' });
        }
        nodes.push({
            type: 'text',
            value: preserveIndent(rawLine)
        });
    }
    if (!nodes.length) {
        nodes.push({ type: 'text', value: '' });
    }
    return nodes;
}
export const remarkLiteralHtml = () => {
    return (tree) => {
        visit(tree, 'html', (node, index, parent) => {
            if (!parent || typeof index !== 'number') {
                return;
            }
            const replacement = createLiteralChildren(node.value);
            if (!PHRASE_PARENTS.has(parent.type)) {
                const paragraph = {
                    type: 'paragraph',
                    children: replacement,
                    data: { literalHtml: true }
                };
                const siblings = parent.children;
                siblings.splice(index, 1, paragraph);
                if (index > 0) {
                    const previous = siblings[index - 1];
                    if (previous?.type === 'paragraph' &&
                        previous.data?.literalHtml) {
                        const prevChildren = previous.children;
                        if (prevChildren.length) {
                            const lastChild = prevChildren[prevChildren.length - 1];
                            if (lastChild.type !== 'break') {
                                prevChildren.push({
                                    type: 'break'
                                });
                            }
                        }
                        prevChildren.push(...paragraph.children);
                        siblings.splice(index, 1);
                        return index;
                    }
                }
                return index + 1;
            }
            parent.children.splice(index, 1, ...replacement);
            return index + replacement.length;
        });
    };
};
