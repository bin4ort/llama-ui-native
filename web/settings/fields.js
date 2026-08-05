/**
 * fields.js — Agent B shared form-field builders for settings sections.
 * Every control binds to kernel settings (updateConfig) and renders with
 * the same markup, so all 8 sections stay visually consistent.
 */
import { t } from '../kernel/index.js';

function row(label, control, help) {
  const wrap = document.createElement('div');
  wrap.className = 'space-y-1.5 py-3 first:pt-0 last:pb-0';
  const lab = document.createElement('label');
  lab.className = 'block text-sm font-medium';
  lab.textContent = label;
  wrap.appendChild(lab);
  if (control) {
    control.className = (control.className || '') + ' mt-1 w-full';
    wrap.appendChild(control);
  }
  if (help) {
    const p = document.createElement('p');
    p.className = 'text-xs text-muted-foreground';
    p.textContent = help;
    wrap.appendChild(p);
  }
  return wrap;
}

function sectionCard(title) {
  const card = document.createElement('div');
  card.className = 'rounded-lg border border-border/60 bg-card p-4';
  if (title) {
    const h = document.createElement('h3');
    h.className = 'text-sm font-semibold mb-2';
    h.textContent = title;
    card.appendChild(h);
  }
  return card;
}

export function checkboxField(label, checked, help, onChange) {
  const box = document.createElement('input');
  box.type = 'checkbox';
  box.checked = !!checked;
  box.className = 'mt-1 h-4 w-4 rounded border-border bg-background';
  box.addEventListener('change', () => onChange(box.checked));
  const wrap = document.createElement('div');
  wrap.className = 'flex items-start gap-3 py-3 first:pt-0 last:pb-0';
  wrap.appendChild(box);
  const text = document.createElement('div');
  text.className = 'space-y-0.5';
  const lab = document.createElement('span');
  lab.className = 'text-sm font-medium block';
  lab.textContent = label;
  text.appendChild(lab);
  if (help) {
    const p = document.createElement('p');
    p.className = 'text-xs text-muted-foreground';
    p.textContent = help;
    text.appendChild(p);
  }
  wrap.appendChild(text);
  return wrap;
}

export function selectField(label, value, options, help, onChange) {
  const sel = document.createElement('select');
  sel.className = 'h-9 rounded-md border border-input bg-background px-3 text-sm';
  for (const [val, text] of options) {
    const o = document.createElement('option');
    o.value = val;
    o.textContent = text;
    o.selected = String(val) === String(value);
    sel.appendChild(o);
  }
  sel.addEventListener('change', () => onChange(sel.value));
  return row(label, sel, help);
}

export function sliderField(label, value, min, max, step, help, onChange, format) {
  const wrap = document.createElement('div');
  wrap.className = 'space-y-1.5 py-3 first:pt-0 last:pb-0';
  const head = document.createElement('div');
  head.className = 'flex items-center justify-between gap-2';
  const lab = document.createElement('label');
  lab.className = 'text-sm font-medium';
  lab.textContent = label;
  head.appendChild(lab);
  const val = document.createElement('span');
  val.className = 'text-xs tabular-nums text-muted-foreground';
  val.textContent = format ? format(value) : String(value);
  head.appendChild(val);
  wrap.appendChild(head);

  const input = document.createElement('input');
  input.type = 'range';
  input.min = String(min);
  input.max = String(max);
  input.step = String(step);
  input.value = String(value);
  input.className = 'w-full accent-primary';
  input.addEventListener('input', () => {
    const v = Number(input.value);
    val.textContent = format ? format(v) : String(v);
    onChange(v);
  });
  wrap.appendChild(input);

  if (help) {
    const p = document.createElement('p');
    p.className = 'text-xs text-muted-foreground';
    p.textContent = help;
    wrap.appendChild(p);
  }
  return wrap;
}

export function textField(label, value, help, onChange, opts = {}) {
  const inp = document.createElement('input');
  inp.type = opts.type || 'text';
  inp.value = value ?? '';
  inp.placeholder = opts.placeholder || '';
  inp.className = 'h-9 rounded-md border border-input bg-background px-3 text-sm';
  inp.addEventListener('input', () => onChange(inp.value));
  return row(label, inp, help);
}

export function button(text, onClick, variant = 'default') {
  const b = document.createElement('button');
  b.type = 'button';
  b.className =
    variant === 'outline'
      ? 'h-9 rounded-md border border-input px-3 text-sm hover:bg-accent'
      : variant === 'ghost'
        ? 'h-9 rounded-md px-3 text-sm text-muted-foreground hover:bg-accent hover:text-foreground'
        : 'h-9 rounded-md bg-primary px-3 text-sm font-medium text-primary-foreground hover:bg-primary/90';
  b.textContent = text;
  b.addEventListener('click', onClick);
  return b;
}

export { row, sectionCard };
