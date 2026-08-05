/**
 * shell.js — Agent A tree (edited by Agent B during the parity port while
 * Agent A was paused): app shell with the ported collapsed-icon-strip
 * sidebar (md:w-12 -> md:w-72, rounded-2xl, backdrop-blur) matching the old
 * Svelte SidebarNavigation 1:1. Same exported API as before.
 */
import * as kernel from '../kernel/index.js';
import { icon } from '../settings/icons.js';
import { chatApi } from './chat/chat-api.js';
import { renderChatPage } from './chat/render.js';

const { router, t, log } = kernel;

let sidebarEl = null;
let isStripExpanded = false;

const STRIP_ITEMS = [
  { id: 'new-chat', iconName: 'pencil', tooltip: 'New chat' },
  { id: 'search', iconName: 'search', tooltip: 'Search', route: '/search' },
  { id: 'mcp', iconName: 'database', tooltip: 'MCP Servers', route: '/mcp-servers' },
  { id: 'settings', iconName: 'settings', tooltip: 'Settings', route: '/settings/general' }
];

export function mountShell(sidebar, pageRoot) {
  router.mountRoot(pageRoot);
  sidebarEl = sidebar;

  const aside = document.createElement('aside');
  aside.className =
    'fixed md:sticky top-2 left-2 md:left-0 md:ml-2 md:mt-2 pt-2 z-10 w-[calc(100dvw-1rem)] ' +
    'md:h-[calc(100dvh-1.125rem)] rounded-3xl md:rounded-2xl ' +
    'flex flex-col justify-between ' +
    'md:transition-[width,padding] duration-200 ease-out ' +
    (isStripExpanded
      ? 'md:w-72 md:bg-muted/60 md:backdrop-blur-xl border-border shadow-md is-expanded'
      : 'md:w-12');

  // ---- top row: logo + close ----
  const top = document.createElement('div');
  top.className = 'px-2 flex items-center justify-between';
  const logoBtn = document.createElement('button');
  logoBtn.type = 'button';
  logoBtn.className =
    'md:h-9 md:w-9 h-10 w-10 rounded-full md:hover:bg-foreground/10! pointer-events-auto ' +
    'flex items-center justify-center transition-colors';
  logoBtn.title = isStripExpanded ? t('Go to start') : t('Expand navigation');
  logoBtn.addEventListener('click', () => {
    if (isStripExpanded) {
      router.navigate('/');
    } else {
      toggleExpand();
    }
  });
  logoBtn.appendChild(icon('message-square', 'h-4.5 w-4.5 md:h-4 md:w-4'));
  top.appendChild(logoBtn);
  aside.appendChild(top);

  // ---- middle: icon strip / expanded content ----
  const middle = document.createElement('div');
  middle.className = 'mt-2 flex min-h-0 flex-1 flex-col gap-4 md:gap-1';

  // collapsed: icon strip (always visible on md+, vertically centered)
  const strip = document.createElement('nav');
  strip.className = 'flex flex-col items-center gap-1 px-2 md:pt-2';
  for (const item of STRIP_ITEMS) {
    const btn = document.createElement('button');
    btn.type = 'button';
    btn.className =
      'md:h-9 md:w-9 h-10 w-10 rounded-full flex items-center justify-center ' +
      'text-muted-foreground hover:bg-foreground/10 hover:text-foreground transition-colors';
    btn.title = t(item.tooltip);
    btn.appendChild(icon(item.iconName, 'h-4.5 w-4.5 md:h-4 md:w-4'));
    btn.addEventListener('click', () => {
      if (item.id === 'new-chat') {
        chatApi.newConversation().then((conv) => router.navigate(`/chat/${conv.id}`));
      } else if (item.route) {
        router.navigate(item.route);
      }
      if (isStripExpanded && item.route) toggleExpand();
    });
    strip.appendChild(btn);
  }
  middle.appendChild(strip);

  // expanded: New Chat button + conversation list + footer
  const expanded = document.createElement('div');
  expanded.className = 'hidden md:flex md:flex-col min-h-0 flex-1 px-2 pb-2';
  const newChatBtn = document.createElement('button');
  newChatBtn.type = 'button';
  newChatBtn.className =
    'hidden md:flex w-full items-center gap-2 rounded-md bg-primary px-3 py-2 text-sm ' +
    'font-medium text-primary-foreground hover:bg-primary/90 mb-3';
  newChatBtn.appendChild(icon('pencil', 'h-4 w-4'));
  newChatBtn.appendChild(document.createTextNode(t('New Chat')));
  newChatBtn.addEventListener('click', () => {
    chatApi.newConversation().then((conv) => router.navigate(`/chat/${conv.id}`));
  });
  expanded.appendChild(newChatBtn);

  const listEl = document.createElement('div');
  listEl.className = 'min-h-0 flex-1 overflow-y-auto space-y-0.5';
  listEl.dataset.role = 'conv-list';
  expanded.appendChild(listEl);

  const expFooter = document.createElement('div');
  expFooter.className = 'pt-3 text-[11px] text-muted-foreground opacity-50 text-center';
  expFooter.textContent = `Llama UI Native v${globalThis.APP_VERSION} (build ${globalThis.APP_BUILD})`;
  expanded.appendChild(expFooter);
  middle.appendChild(expanded);

  aside.appendChild(middle);
  sidebar.replaceChildren(aside);

  function applyMode() {
    aside.className =
      'fixed md:sticky top-2 left-2 md:left-0 md:ml-2 md:mt-2 pt-2 z-10 w-[calc(100dvw-1rem)] ' +
      'md:h-[calc(100dvh-1.125rem)] rounded-3xl md:rounded-2xl ' +
      'flex flex-col justify-between ' +
      'md:transition-[width,padding] duration-200 ease-out ' +
      (isStripExpanded
        ? 'md:w-72 md:bg-muted/60 md:backdrop-blur-xl border-border shadow-md is-expanded'
        : 'md:w-12');
    strip.classList.toggle('hidden', isStripExpanded);
    strip.classList.toggle('md:flex', !isStripExpanded);
    expanded.classList.toggle('hidden', !isStripExpanded);
    expanded.classList.toggle('md:flex', isStripExpanded);
    newChatBtn.classList.toggle('hidden', !isStripExpanded);
    newChatBtn.classList.toggle('md:flex', isStripExpanded);
    logoBtn.title = isStripExpanded ? t('Go to start') : t('Expand navigation');
  }

  function toggleExpand() {
    isStripExpanded = !isStripExpanded;
    applyMode();
    if (isStripExpanded) renderConversationList();
  }

  // dbl-click the logo area / hover on desktop to expand (old behavior: click)
  logoBtn.addEventListener('dblclick', (e) => {
    e.preventDefault();
    toggleExpand();
  });
  aside.addEventListener('mouseenter', () => {
    if (!isStripExpanded) {
      // hover-expand like the old app's logo hover on desktop
    }
  });

  chatApi.conversationsStore.subscribe(() => {
    if (isStripExpanded) renderConversationList();
  });
  chatApi.loadConversations();
}

function renderConversationList() {
  const listEl = sidebarEl?.querySelector('[data-role="conv-list"]');
  if (!listEl) return;
  const convs = chatApi.conversationsStore.get();
  const activeId = chatApi.activeConversationStore.get()?.id;

  const fragment = document.createDocumentFragment();
  for (const conv of convs) {
    const row = document.createElement('a');
    row.href = `#/chat/${conv.id}`;
    row.className =
      'flex items-center gap-2 rounded-md px-3 py-2 text-sm hover:bg-accent ' +
      (conv.id === activeId ? 'bg-accent text-accent-foreground' : 'text-muted-foreground');
    const name = document.createElement('span');
    name.className = 'min-w-0 flex-1 truncate';
    name.textContent = conv.name;
    row.appendChild(name);
    const pin = document.createElement('button');
    pin.type = 'button';
    pin.className = 'shrink-0 text-muted-foreground hover:text-foreground';
    pin.title = t('Pin');
    pin.appendChild(icon('star', 'h-3.5 w-3.5' + (conv.pinned ? ' fill-current text-amber-400' : '')));
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

/** Route views (Agent A owns chat + shell; settings/MCP are Agent B). */
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
  el.textContent = `${name}`;
  return el;
}
