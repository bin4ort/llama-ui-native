/**
 * shell.js — Agent A: app shell. Sidebar (conversation list, actions) +
 * page-root mount + boot application of theme/locale.
 */
import * as kernel from '../kernel/index.js';
import { chatApi } from './chat/chat-api.js';
import { renderChatPage } from './chat/render.js';

const { router, t, theme, settings, log } = kernel;

let sidebarEl = null;

export function mountShell(sidebar, pageRoot) {
  router.mountRoot(pageRoot);
  sidebarEl = sidebar;
  sidebar.innerHTML = `
    <div class="flex h-full flex-col">
      <div class="flex items-center gap-2 px-4 py-4">
        <span class="text-sm font-semibold">Llama UI</span>
      </div>
      <div class="px-2 pb-2">
        <button class="w-full rounded-md bg-primary px-3 py-2 text-sm font-medium text-primary-foreground hover:bg-primary/90" data-action="new-chat">
          ${t('New Chat')}
        </button>
      </div>
      <nav class="flex flex-col gap-1 px-2 text-sm">
        <a href="#/" class="px-3 py-2 rounded-md hover:bg-accent" data-route="/">${t('Chat')}</a>
        <a href="#/search" class="px-3 py-2 rounded-md hover:bg-accent">${t('Search')}</a>
        <a href="#/mcp-servers" class="px-3 py-2 rounded-md hover:bg-accent">MCP Servers</a>
        <a href="#/settings/general" class="px-3 py-2 rounded-md hover:bg-accent">${t('Settings')}</a>
      </nav>
      <div class="mt-2 flex-1 overflow-y-auto border-t border-border/40 px-2 py-2" data-role="conv-list"></div>
      <div class="px-4 py-3 text-[11px] text-muted-foreground">
        Llama UI v${globalThis.APP_VERSION} (build ${globalThis.APP_BUILD})
      </div>
    </div>`;

  sidebar.querySelector('[data-action="new-chat"]').addEventListener('click', () => {
    chatApi.newConversation().then((conv) => {
      router.navigate(`/chat/${conv.id}`);
    });
  });

  chatApi.conversationsStore.subscribe(() => renderConversationList());
  chatApi.loadConversations();
}

function renderConversationList() {
  const listEl = sidebarEl.querySelector('[data-role="conv-list"]');
  const convs = chatApi.conversationsStore.get();
  const activeId = chatApi.activeConversationStore.get()?.id;

  const fragment = document.createDocumentFragment();
  for (const conv of convs) {
    const row = document.createElement('a');
    row.href = `#/chat/${conv.id}`;
    row.className =
      'flex items-center gap-2 rounded-md px-3 py-2 text-sm hover:bg-accent ' +
      (conv.id === activeId ? 'bg-accent text-accent-foreground' : '');
    row.textContent = conv.name;
    row.addEventListener('click', () => {
      router.navigate(`/chat/${conv.id}`);
    });
    const pin = document.createElement('button');
    pin.className = 'ml-auto text-muted-foreground hover:text-foreground';
    pin.textContent = conv.pinned ? '📌' : '';
    pin.title = t('Pin');
    pin.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      chatApi.togglePin(conv.id);
    });
    row.appendChild(pin);
    fragment.appendChild(row);
  }

  if (convs.length === 0) {
    const empty = document.createElement('div');
    empty.className = 'px-3 py-4 text-center text-xs text-muted-foreground';
    empty.textContent = t('No conversations yet');
    fragment.appendChild(empty);
  }

  listEl.replaceChildren(fragment);
}

/** Route views (Agent A owns chat + shell; settings/MCP are Agent B stubs). */
export function renderChat(params) {
  const id = params[0];
  const container = document.createElement('div');
  container.className = 'h-full';
  if (id && id !== 'new') {
    chatApi.openConversation(id).then(() => renderChatPage(container));
  } else {
    chatApi.newConversation().then(() => renderChatPage(container));
  }
  return container;
}

export function renderSearch() {
  return import('./search.js').then((m) => m.renderSearchPage());
}

export function placeholder(name) {
  const el = document.createElement('div');
  el.className = 'flex h-full items-center justify-center p-8 text-muted-foreground';
  el.textContent = `${name} — Agent B vertical (Phase 2)`;
  return el;
}
