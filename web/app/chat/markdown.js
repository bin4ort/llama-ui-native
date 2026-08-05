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
