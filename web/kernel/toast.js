/**
 * toast.js — minimal toast primitive (svelte-sonner-like).
 */
let host = null;

export function mountToasts(el) {
  host = el;
}

export function toast(message, type = 'default', duration = 3000) {
  if (!host) return;
  const node = document.createElement('div');
  node.className =
    'pointer-events-auto rounded-lg border border-border/50 bg-background px-4 py-2 text-sm shadow-md ' +
    (type === 'error' ? 'text-destructive' : '');
  node.textContent = message;
  host.appendChild(node);
  setTimeout(() => {
    node.style.opacity = '0';
    node.style.transition = 'opacity 200ms';
    setTimeout(() => node.remove(), 220);
  }, duration);
}

export const toastApi = {
  success: (msg) => toast(msg, 'default'),
  error: (msg) => toast(msg, 'error'),
  info: (msg) => toast(msg, 'default')
};
