/**
 * markdown.js — Agent A: markdown pipeline (marked + DOMPurify + GFM).
 * Lazy post-processing: highlight.js (code), mermaid (diagrams), katex
 * (math, $...$ inline / $$...$$ display). Error codes: LLMUI-MD-*.
 */
import { marked } from 'marked';
import DOMPurify from 'dompurify';
import { log } from '../../kernel/index.js';

marked.setOptions({
  gfm: true,
  breaks: false
});

/* Math is protected from markdown parsing with hex placeholders, then
 * rendered with katex in the post-processor. */
const MATH_RE = /\$\$([\s\S]+?)\$\$|\$([^$\n]+?)\$/g;

function hexEncode(value) {
  return [...new TextEncoder().encode(value)].map((b) => b.toString(16).padStart(2, '0')).join('');
}

function hexDecode(value) {
  const bytes = new Uint8Array(value.length / 2);
  for (let i = 0; i < bytes.length; i++) bytes[i] = parseInt(value.slice(i * 2, i * 2 + 2), 16);
  return new TextDecoder().decode(bytes);
}

function protectMath(source) {
  return source.replace(MATH_RE, (m, blockTex, inlineTex) => {
    const tex = blockTex ?? inlineTex;
    return `§LLMUI_MATH§${hexEncode(tex)}§${blockTex ? 'B' : 'I'}§`;
  });
}

export function renderMarkdown(source) {
  if (!source) return '';
  try {
    const html = marked.parse(protectMath(source), { async: false });
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

let katexCssInjected = false;
async function ensureKatexCss() {
  if (katexCssInjected) return;
  katexCssInjected = true;
  const link = document.createElement('link');
  link.rel = 'stylesheet';
  link.href = '/assets/katex/katex.min.css';
  document.head.appendChild(link);
}

/**
 * Post-process rendered HTML inside a container: katex placeholders,
 * mermaid diagrams and code highlighting (all lazy-loaded chunks).
 */
export async function renderExtras(root) {
  if (!root) return;

  // katex placeholders
  const placeholderRe = /§LLMUI_MATH§([0-9a-f]+)§([BI])§/;
  let katex = null;
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  const textNodes = [];
  while (walker.nextNode()) {
    const node = walker.currentNode;
    if (node.nodeValue && placeholderRe.test(node.nodeValue)) textNodes.push(node);
  }
  if (textNodes.length) await ensureKatexCss();
  for (const node of textNodes) {
    try {
      katex ??= (await import('katex')).default;
      const frag = document.createDocumentFragment();
      for (const part of node.nodeValue.split(/(§LLMUI_MATH§[0-9a-f]+§[BI]§)/)) {
        const m = part.match(placeholderRe);
        if (m) {
          const wrapper = document.createElement('span');
          wrapper.innerHTML = katex.renderToString(hexDecode(m[1]), {
            displayMode: m[2] === 'B',
            throwOnError: false
          });
          frag.appendChild(wrapper);
        } else if (part) {
          frag.appendChild(document.createTextNode(part));
        }
      }
      node.replaceWith(frag);
    } catch (err) {
      log.warn('LLMUI-MD-001', 'markdown: latex render failed', err?.message ?? String(err));
    }
  }

  // mermaid diagrams
  const blocks = root.querySelectorAll('pre code.language-mermaid');
  if (blocks.length) {
    try {
      const mermaid = (await import('mermaid')).default;
      mermaid.initialize({ startOnLoad: false, theme: 'default', securityLevel: 'strict' });
      for (const block of blocks) {
        try {
          const id = `mmd_${Math.random().toString(36).slice(2, 10)}`;
          const { svg } = await mermaid.render(id, block.textContent);
          const pre = block.closest('pre');
          if (pre) pre.outerHTML = `<div class="mermaid-block overflow-x-auto rounded-lg bg-background p-2">${svg}</div>`;
        } catch (err) {
          log.warn('LLMUI-MD-002', 'markdown: mermaid render failed', err?.message ?? String(err));
        }
      }
    } catch (err) {
      log.warn('LLMUI-MD-002', 'markdown: mermaid load failed', err?.message ?? String(err));
    }
  }

  // code highlighting
  const codeBlocks = root.querySelectorAll('pre code');
  if (codeBlocks.length) {
    try {
      const hljs = (await import('highlight.js')).default;
      for (const block of codeBlocks) {
        if (block.className.includes('language-mermaid')) continue;
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
}
