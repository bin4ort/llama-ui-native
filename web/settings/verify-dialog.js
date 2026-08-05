/**
 * verify-dialog.js — Agent B: VerificationDialog (kernel permissions
 * contract). Registered via kernel.permissions.registerVerifier() at boot;
 * called by Agent A's tool-call flow with { title, description, confirmText,
 * cancelText }. Resolves true/false.
 */
import { t, showModal, log, permissions } from '../kernel/index.js';

function buildDialog(request, resolve) {
  const body = document.createElement('div');
  body.className = 'space-y-4';

  if (request.description) {
    const p = document.createElement('p');
    p.className = 'text-sm';
    p.textContent = request.description;
    body.appendChild(p);
  }

  const row = document.createElement('div');
  row.className = 'flex items-center justify-end gap-2 pt-2';

  const deny = document.createElement('button');
  deny.type = 'button';
  deny.className = 'h-9 rounded-md border border-input px-3 text-sm hover:bg-accent';
  deny.textContent = request.cancelText || t('Deny');
  deny.addEventListener('click', () => {
    resolve(false);
    handle.close();
  });
  row.appendChild(deny);

  const allow = document.createElement('button');
  allow.type = 'button';
  allow.className = 'h-9 rounded-md bg-primary px-3 text-sm font-medium text-primary-foreground hover:bg-primary/90';
  allow.textContent = request.confirmText || t('Allow once');
  allow.addEventListener('click', () => {
    resolve(true);
    handle.close();
  });
  row.appendChild(allow);

  body.appendChild(row);

  const handle = showModal({
    title: request.title || t('Permission required'),
    content: () => body
  });
  return handle;
}

export function registerVerificationDialog() {
  permissions.registerVerifier(
    (request) =>
      new Promise((resolve) => {
        const handle = buildDialog(request, resolve);
        // If the modal is replaced/closed without a button press, deny.
        const origClose = handle.close;
        handle.close = () => {
          resolve(false);
          origClose();
        };
      })
  );
  log.info('LLMUI-TL-003', 'verification: dialog registered');
}
