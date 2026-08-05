/**
 * modal.js — minimal modal primitive (port of the shadcn-svelte dialog
 * behavior; classes match the current app's Dialog.Content styling).
 */
import { log } from './logger.js';

let openModal = null;
const subs = new Set();

export function modalState() {
  return openModal;
}

export function subscribeModal(fn) {
  subs.add(fn);
  fn(openModal);
  return () => subs.delete(fn);
}

function notify() {
  for (const fn of subs) fn(openModal);
}

/**
 * openModal({ title, description, content, onClose }) — content is a
 * function returning a Node (or a Promise). Returns a close handle.
 */
export function showModal(spec) {
  if (openModal) {
    log.warn('LLMUI-UI-000', 'ui: modal already open — replacing');
  }
  openModal = spec;
  notify();
  return {
    close: () => {
      if (openModal === spec) {
        openModal = null;
        notify();
        spec.onClose?.();
      }
    }
  };
}

export function closeModal() {
  if (openModal) {
    const onClose = openModal.onClose;
    openModal = null;
    notify();
    onClose?.();
  }
}

/**
 * Mount the modal renderer into a host element. Subscribes to the modal
 * state and renders the open modal (overlay + centered dialog) — shared by
 * all dialogs built on showModal (verification, preset picker, …).
 */
export function mountModalHost(host) {
  subscribeModal((spec) => {
    host.replaceChildren();
    if (!spec) return;

    const overlay = document.createElement('div');
    overlay.className =
      'fixed inset-0 z-50 flex items-center justify-center bg-black/50 p-4';
    overlay.addEventListener('click', (e) => {
      if (e.target === overlay) closeModal();
    });

    const dialog = document.createElement('div');
    dialog.className =
      'w-full max-w-xl rounded-lg border border-border bg-card p-5 shadow-xl max-h-[85vh] overflow-y-auto';
    const title = document.createElement('h3');
    title.className = 'text-base font-semibold mb-1';
    title.textContent = spec.title || '';
    dialog.appendChild(title);
    if (spec.description) {
      const d = document.createElement('p');
      d.className = 'mb-4 text-sm text-muted-foreground';
      d.textContent = spec.description;
      dialog.appendChild(d);
    }
    const content = spec.content?.();
    if (content) dialog.appendChild(content);

    overlay.appendChild(dialog);
    host.appendChild(overlay);
  });
}
