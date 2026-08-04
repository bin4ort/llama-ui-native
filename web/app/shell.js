/**
 * shell.js — Agent A: app shell (sidebar + page-root mount + theme/locale
 * application on boot). The chat vertical builds on this file's structure.
 */
import * as kernel from '../kernel/index.js';

const { router, t, theme, settings, log } = kernel;

export function mountShell(sidebarEl, pageRootEl) {
  router.mountRoot(pageRootEl);
  renderSidebar(sidebarEl);
}

function renderSidebar(el) {
  el.innerHTML = `
    <div class="flex h-full flex-col">
      <div class="flex items-center gap-2 px-4 py-4">
        <span class="text-sm font-semibold">Llama UI</span>
      </div>
      <nav class="flex flex-col gap-1 px-2 text-sm">
        <a href="#/" class="sidebar-link px-3 py-2 rounded-md hover:bg-accent" data-route="/">Chat</a>
        <a href="#/search" class="sidebar-link px-3 py-2 rounded-md hover:bg-accent">Search</a>
        <a href="#/mcp-servers" class="sidebar-link px-3 py-2 rounded-md hover:bg-accent">MCP Servers</a>
        <a href="#/settings/general" class="sidebar-link px-3 py-2 rounded-md hover:bg-accent">Settings</a>
      </nav>
      <div class="mt-auto px-4 py-3 text-[11px] text-muted-foreground">
        Llama UI v${APP_VERSION} (build ${APP_BUILD})
      </div>
    </div>`;
}

/** Route handlers live in each vertical; this is the Phase-1 placeholder chat. */
export function renderChatEmpty() {
  const root = document.createElement('div');
  root.className = 'flex h-full flex-col items-center justify-center gap-3 px-4';
  root.innerHTML = `
    <h1 class="text-2xl font-semibold">${t('Welcome to Llama UI')}</h1>
    <p class="text-muted-foreground">${t('Start a conversation with your local AI')}</p>
    <a href="#/chat/new" class="mt-2 rounded-md bg-primary px-4 py-2 text-sm font-medium text-primary-foreground hover:bg-primary/90">
      ${t('Start a new chat')}
    </a>`;
  return root;
}
