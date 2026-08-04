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
