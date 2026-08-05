/**
 * render.js — Agent A: chat page rendering (message list, streaming bubble,
 * reasoning block, tool-call row) + auto-scroll. Re-renders the list on
 * store changes (simple and correct; virtualized later if needed).
 */
import * as kernel from '../../kernel/index.js';
import { messagesStore, streamingStore, streamContentStore } from './chat.js';
import { renderMarkdown } from './markdown.js';

const { t } = kernel;

let root = null;
let scrolled = true;

export function renderChatPage(container) {
  root = document.createElement('div');
  root.className = 'flex h-full flex-col';
  root.innerHTML = `
    <div class="flex-1 overflow-y-auto px-4" data-role="messages"></div>
    <div data-role="composer" class="border-t border-border/50 p-4"></div>`;
  container.replaceChildren(root);

  const listEl = root.querySelector('[data-role="messages"]');
  listEl.addEventListener('scroll', () => {
    scrolled = listEl.scrollHeight - listEl.scrollTop - listEl.clientHeight < 60;
  });

  const composer = renderComposer();
  root.querySelector('[data-role="composer"]').replaceChildren(composer);

  messagesStore.subscribe(() => renderList(listEl));
  streamingStore.subscribe(() => renderList(listEl));
  streamContentStore.subscribe(() => {
    // update only the streaming bubble content without a full re-render
    const bubble = listEl.querySelector('[data-streaming] .message-content');
    if (bubble) bubble.innerHTML = renderMarkdown(streamContentStore.get());
    scrollToBottom(listEl);
  });

  renderList(listEl);
  return root;
}

function renderList(listEl) {
  const msgs = messagesStore.get();
  const streaming = streamingStore.get();
  const fragment = document.createDocumentFragment();
  for (const m of msgs) {
    fragment.appendChild(renderMessage(m, streaming));
  }
  listEl.replaceChildren(fragment);
  if (scrolled) scrollToBottom(listEl);
}

function scrollToBottom(listEl) {
  listEl.scrollTop = listEl.scrollHeight;
}

function renderMessage(m, streaming) {
  const wrap = document.createElement('div');
  wrap.className = 'mx-auto my-4 max-w-3xl';

  if (m.type === 'persona' || m.type === 'system') {
    wrap.innerHTML = `
      <div class="rounded-2xl border-2 border-dashed border-border/50 bg-muted px-3 py-1.5 text-sm">
        <div class="message-content whitespace-pre-wrap">${escapeHtml(m.content ?? '')}</div>
      </div>`;
    return wrap;
  }

  if (m.type === 'tool') {
    wrap.innerHTML = `
      <div class="rounded-lg border border-border/40 bg-muted/40 px-3 py-2 text-xs font-mono whitespace-pre-wrap">
        <div class="mb-1 text-[10px] uppercase tracking-wide text-muted-foreground">${t('Tool calls')}</div>
        ${escapeHtml((m.content ?? '').slice(0, 2000))}
      </div>`;
    return wrap;
  }

  const isAssistant = m.role === 'assistant';
  const isStreaming = streaming && isAssistant && m === messagesStore.get().at(-1);

  let body = '';
  if (isStreaming) {
    body = `<div class="message-content">${renderMarkdown(streamContentStore.get() || '…')}</div>`;
  } else if (m.reasoning) {
    body = `
      <details class="mb-2 rounded-lg bg-muted/40 px-3 py-2 text-sm text-muted-foreground">
        <summary class="cursor-pointer text-xs uppercase tracking-wide">${t('Reasoning')}</summary>
        <div class="mt-2 whitespace-pre-wrap">${escapeHtml(m.reasoning)}</div>
      </details>
      <div class="message-content">${renderMarkdown(m.content ?? '')}</div>`;
  } else {
    body = `<div class="message-content">${renderMarkdown(m.content ?? '')}</div>`;
  }

  wrap.className += ` flex ${isAssistant ? 'justify-start' : 'justify-end'}`;
  wrap.innerHTML = `
    <div class="max-w-[80%] rounded-2xl px-3 py-2 text-sm ${isAssistant ? 'bg-muted/60' : 'bg-primary text-primary-foreground'}" ${isStreaming ? 'data-streaming=""' : ''}>
      ${body}
    </div>`;

  if (m.toolCalls?.length && !isStreaming) {
    for (const tc of m.toolCalls) {
      const row = document.createElement('div');
      row.className = 'mt-1 rounded bg-background/60 px-2 py-1 font-mono text-xs';
      row.textContent = `${tc.function?.name ?? ''}(${(tc.function?.arguments ?? '').slice(0, 120)})`;
      wrap.querySelector('div').appendChild(row);
    }
  }

  return wrap;
}

function renderComposer() {
  const wrap = document.createElement('div');
  wrap.className = 'mx-auto flex max-w-3xl items-end gap-2';
  wrap.innerHTML = `
    <textarea
      rows="1"
      class="min-h-[40px] w-full resize-none rounded-xl border border-input bg-background px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-ring"
      placeholder="${t('Type a message...')}"
    ></textarea>
    <button class="shrink-0 rounded-xl bg-primary px-4 py-2 text-sm font-medium text-primary-foreground hover:bg-primary/90 disabled:opacity-50"
      data-role="send">${t('Send')}</button>`;

  const textarea = wrap.querySelector('textarea');
  const sendBtn = wrap.querySelector('[data-role="send"]');
  const { sendMessage, abortStream, streamingStore: st } = chatApi;

  function updateState() {
    const busy = st.get();
    sendBtn.textContent = busy ? t('Stop') : t('Send');
    sendBtn.disabled = !busy && !textarea.value.trim();
  }
  st.subscribe(updateState);
  textarea.addEventListener('input', () => {
    textarea.style.height = 'auto';
    textarea.style.height = Math.min(textarea.scrollHeight, 200) + 'px';
    updateState();
  });
  textarea.addEventListener('keydown', (e) => {
    if (e.key === 'Enter' && !e.shiftKey) {
      e.preventDefault();
      submit();
    }
  });
  sendBtn.addEventListener('click', () => {
    if (st.get()) abortStream();
    else submit();
  });

  function submit() {
    const value = textarea.value.trim();
    if (!value || st.get()) return;
    textarea.value = '';
    textarea.style.height = 'auto';
    updateState();
    sendMessage(value).catch(() => {});
  }

  return wrap;
}

import { chatApi } from './chat-api.js';

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}
