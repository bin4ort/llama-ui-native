/**
 * search.js — Agent A: search page (full-text over conversations/messages
 * via the kernel db). Simple includes-match; port the indexer later.
 * Error codes: LLMUI-DB-*.
 */
import * as kernel from '../kernel/index.js';
import { log } from '../kernel/index.js';

const { t, db } = kernel;

export async function renderSearchPage() {
  const container = document.createElement('div');
  container.className = 'mx-auto max-w-3xl p-6';
  container.innerHTML = `
    <h1 class="mb-4 text-xl font-semibold">${t('Search')}</h1>
    <input class="mb-4 w-full rounded-md border border-input bg-background px-3 py-2 text-sm outline-none focus:ring-2 focus:ring-ring" placeholder="${t('Search conversations...')}" data-role="q" />
    <div data-role="results" class="space-y-2"></div>`;

  const q = container.querySelector('[data-role="q"]');
  const results = container.querySelector('[data-role="results"]');

  q.addEventListener('input', () => run(q.value));

  async function run(query) {
    const needle = query.trim().toLowerCase();
    results.replaceChildren();
    if (!needle) return;
    try {
      const convs = await db.listConversations();
      const byId = new Map(convs.map((c) => [c.id, c]));
      const rows = [];
      for (const conv of convs) {
        const msgs = await db.getMessagesByConversation(conv.id);
        const hit = msgs.find(
          (m) => typeof m.content === 'string' && m.content.toLowerCase().includes(needle)
        );
        if (hit) {
          rows.push({ conv, snippet: hit.content.slice(0, 160) });
        }
      }
      if (rows.length === 0) {
        const empty = document.createElement('p');
        empty.className = 'py-4 text-center text-sm text-muted-foreground';
        empty.textContent = t('No results found');
        results.appendChild(empty);
        return;
      }
      for (const { conv, snippet } of rows) {
        const a = document.createElement('a');
        a.href = `#/chat/${conv.id}`;
        a.className = 'block rounded-lg border border-border/40 px-3 py-2 hover:bg-accent';
        a.innerHTML = `<div class="text-sm font-medium">${escapeHtml(conv.name)}</div>
          <div class="mt-0.5 text-xs text-muted-foreground">${escapeHtml(snippet)}</div>`;
        results.appendChild(a);
      }
    } catch (err) {
      log.error('LLMUI-DB-008', 'search: query failed', String(err));
    }
  }

  return container;
}

function escapeHtml(value) {
  return String(value)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}
