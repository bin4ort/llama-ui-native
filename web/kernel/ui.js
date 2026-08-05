/**
 * ui.js — kernel UI primitives (port of the shadcn-svelte button/checkbox
 * behavior; classes match the current app). Kernel-owned; verticals consume.
 */

/** Button factory: button(label, onclick, { variant, size, className, disabled }) */
export function button(label, onclick, options = {}) {
  const el = document.createElement('button');
  el.type = 'button';
  el.textContent = label;
  const variant = options.variant ?? 'default';
  const size = options.size ?? 'default';
  const classes = {
    default: 'bg-primary text-primary-foreground hover:bg-primary/90',
    outline: 'border border-input hover:bg-accent',
    ghost: 'hover:bg-accent',
    destructive: 'bg-destructive text-white hover:bg-destructive/80',
    secondary: 'bg-secondary text-secondary-foreground hover:bg-secondary/80'
  };
  const sizes = {
    default: 'h-9 px-4',
    sm: 'h-8 px-3 text-xs',
    lg: 'h-10 px-6'
  };
  el.className = `inline-flex items-center justify-center gap-1.5 rounded-md text-sm font-medium transition-colors disabled:opacity-50 disabled:pointer-events-none ${classes[variant] ?? classes.default} ${sizes[size] ?? sizes.default} ${options.className ?? ''}`;
  el.disabled = Boolean(options.disabled);
  el.addEventListener('click', (e) => {
    if (!el.disabled) onclick?.(e);
  });
  return el;
}

/** Checkbox factory: checkbox(checked, onchange) */
export function checkbox(checked, onchange) {
  const el = document.createElement('input');
  el.type = 'checkbox';
  el.className = 'h-4 w-4 rounded border-input accent-primary';
  el.checked = Boolean(checked);
  el.addEventListener('change', () => onchange?.(el.checked));
  return el;
}

/** Simple collapsible: returns { root, header, content, open } */
export function collapsible({ title, initialOpen = false, headerClass = '' }) {
  const root = document.createElement('div');
  root.className = 'rounded-lg';
  const header = document.createElement('button');
  header.type = 'button';
  header.className = `flex w-full items-center gap-2 rounded-lg px-3 py-2 text-sm hover:bg-muted/50 ${headerClass}`;
  header.textContent = title;
  const content = document.createElement('div');
  content.className = 'ml-4 border-l border-border/50 pl-2';
  content.style.display = initialOpen ? '' : 'none';
  let open = initialOpen;
  header.addEventListener('click', () => {
    open = !open;
    content.style.display = open ? '' : 'none';
    header.classList.toggle('font-medium', open);
  });
  root.append(header, content);
  return { root, header, content, toggle: () => header.click(), isOpen: () => open };
}
