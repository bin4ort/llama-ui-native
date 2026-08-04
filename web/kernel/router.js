/**
 * router.js — hash router.
 * Routes: #/ , #/chat/new , #/chat/{id} , #/search , #/settings/{section} ,
 * #/mcp-servers . Renders the matched view into the page-root slot.
 */
import { log } from './logger.js';

const routes = [];
let current = null;
let pageRoot = null;

export function mountRoot(el) {
  pageRoot = el;
}

export function register(pattern, handler) {
  routes.push({ pattern, handler });
}

export function navigate(hash) {
  const target = hash.startsWith('#') ? hash.slice(1) : hash;
  for (const { pattern, handler } of routes) {
    const match = pattern.exec(target);
    if (match) {
      current = { pattern, handler, params: match.slice(1) };
      render();
      return;
    }
  }
  log.warn('LLMUI-SYS-002', 'router: no route handler matched', target);
}

export function getCurrent() {
  return current;
}

function render() {
  if (!pageRoot || !current) return;
  const el = current.handler(current.params);
  pageRoot.replaceChildren(el);
}

export function init() {
  window.addEventListener('hashchange', () => navigate(window.location.hash));
  navigate(window.location.hash || '#/');
}
