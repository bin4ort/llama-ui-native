/**
 * markdown.js — Agent A: markdown pipeline (marked + DOMPurify + GFM).
 * Lazy chunks for mermaid/katex land with their features.
 * Error codes: LLMUI-MD-*.
 */
import { marked } from 'marked';
import DOMPurify from 'dompurify';
import { log } from '../../kernel/index.js';

marked.setOptions({
  gfm: true,
  breaks: false
});

export function renderMarkdown(source) {
  if (!source) return '';
  try {
    const html = marked.parse(source, { async: false });
    return DOMPurify.sanitize(html);
  } catch (err) {
    log.warn('LLMUI-MD-000', 'markdown: marked parse failed', err?.message ?? String(err));
    const div = document.createElement('div');
    div.className = 'whitespace-pre-wrap';
    div.textContent = source;
    return div.outerHTML;
  }
}

export function renderInlineMarkdown(source) {
  return renderMarkdown(source);
}

/**
 * Post-process rendered HTML: highlight <pre><code> blocks with
 * highlight.js (lazy chunk). Call after inserting the HTML into the DOM.
 */
export async function highlightCode(root) {
  const blocks = root.querySelectorAll('pre code');
  if (!blocks.length) return;
  try {
    const hljs = (await import('highlight.js')).default;
    for (const block of blocks) {
      const langMatch = (block.className.match(/language-(\w+)/) ?? [])[1];
      if (langMatch && hljs.getLanguage(langMatch)) {
        block.innerHTML = hljs.highlight(block.textContent, { language: langMatch }).value;
      } else if (block.textContent.trim()) {
        block.innerHTML = hljs.highlightAuto(block.textContent).value;
      }
      block.classList.add('hljs');
    }
  } catch (err) {
    log.warn('LLMUI-MD-004', 'markdown: highlight failed', err?.message ?? String(err));
  }
}

