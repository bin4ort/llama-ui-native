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
  // Parity: the old settings render fields as plain groups (h3 + space-y-8),
  // not bordered cards.
  const card = document.createElement('div');
  card.className = 'space-y-6';
  if (title) {
    const h = document.createElement('h3');
    h.className = 'mb-6 text-base font-semibold';
    h.textContent = title;
    card.appendChild(h);
  }
  return card;
}

export function checkboxField(label, checked, help, onChange) {
  const box = document.createElement('input');
  box.type = 'checkbox';
  box.checked = !!checked;
  box.className = 'mt-1 h-4 w-4 rounded border-border/30 bg-muted/60 dark:bg-muted/75 shadow-sm';
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
  sel.className = 'bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm outline-none text-foreground rounded-md px-3 py-2 text-sm';
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
  inp.className = 'w-full bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm outline-none text-foreground rounded-md px-3 py-2 text-sm';
  inp.addEventListener('input', () => onChange(inp.value));
  return row(label, inp, help);
}

/**
 * Registry-driven field renderer — ports the old app's field markup 1:1:
 *   <div class="flex items-center gap-2"><label class="flex items-center
 *     gap-1.5 text-sm font-medium">…</label></div>
 *   <input class="w-full"> / <textarea class="min-h-[10rem] w-full
 *     md:max-w-3xl"> / checkbox row
 *   <p class="mt-1 text-xs text-muted-foreground">help</p>
 * Optionally shows a "Default: …" placeholder and a reset button when the
 * value differs from the server prop (currentModelParams).
 */
export function registrySection(fields, opts = {}) {
  const { config, updateConfig, props = {}, translate = (s) => s } = opts;
  const root = document.createElement('div');
  root.className = 'space-y-1.5';

  for (const field of fields) {
    const wrap = document.createElement('div');
    wrap.className = 'space-y-1.5 py-3 first:pt-0 last:pb-0';

    const serverDefault = props[field.key];
    const current = config()[field.key];
    const currentStr = String(current ?? '');
    const isCustom =
      serverDefault != null &&
      currentStr !== '' &&
      Math.round((parseFloat(currentStr) || 0) * 1e6) / 1e6 !==
        Math.round(serverDefault * 1e6) / 1e6;

    if (field.type === 'checkbox') {
      const box = document.createElement('input');
      box.type = 'checkbox';
      box.checked = !!current;
      box.className = 'h-4 w-4 rounded border-border/30 bg-muted/60 dark:bg-muted/75 shadow-sm';
      box.addEventListener('change', () => updateConfig({ [field.key]: box.checked }));
      const labelRow = document.createElement('div');
      labelRow.className = 'flex items-center gap-2';
      const lab = document.createElement('label');
      lab.className = 'flex items-center gap-1.5 text-sm font-medium';
      lab.textContent = translate(field.label);
      labelRow.appendChild(lab);
      wrap.appendChild(labelRow);
      wrap.appendChild(box);
    } else if (field.type === 'radio') {
      const labelRow = document.createElement('div');
      labelRow.className = 'flex items-center gap-2';
      const lab = document.createElement('label');
      lab.className = 'flex items-center gap-1.5 text-sm font-medium';
      lab.textContent = translate(field.label);
      labelRow.appendChild(lab);
      wrap.appendChild(labelRow);
      const radios = document.createElement('div');
      radios.className = 'space-y-1.5';
      for (const [val, text] of field.options || []) {
        const rlab = document.createElement('label');
        rlab.className = 'flex cursor-pointer items-center gap-2 text-sm select-none';
        const r = document.createElement('input');
        r.type = 'radio';
        r.name = field.key;
        r.value = val;
        r.checked = String(current ?? '') === val;
        r.className = 'h-4 w-4 accent-primary';
        r.addEventListener('change', () => {
          if (r.checked) {
            updateConfig({ [field.key]: val === 'true', titleGenerationUseLLM: val !== 'true' });
          }
        });
        rlab.appendChild(r);
        const span = document.createElement('span');
        span.textContent = translate(text);
        rlab.appendChild(span);
        radios.appendChild(rlab);
      }
      wrap.appendChild(radios);
    } else {
      const labelRow = document.createElement('div');
      labelRow.className = 'flex items-center gap-2';
      const lab = document.createElement('label');
      lab.className = 'flex items-center gap-1.5 text-sm font-medium';
      lab.textContent = translate(field.label);
      labelRow.appendChild(lab);
      wrap.appendChild(labelRow);

      if (field.type === 'select') {
        const sel = document.createElement('select');
        sel.className = 'w-full bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm outline-none text-foreground rounded-md px-3 py-2 text-sm';
        const options = opts.selectOptions?.[field.key] || field.options || [];
        for (const [val, text] of options) {
          const o = document.createElement('option');
          o.value = val;
          o.textContent = text;
          o.selected = String(val) === currentStr;
          sel.appendChild(o);
        }
        sel.addEventListener('change', () => updateConfig({ [field.key]: sel.value }));
        wrap.appendChild(sel);
      } else if (field.type === 'textarea') {
        const ta = document.createElement('textarea');
        ta.className = 'min-h-[10rem] w-full md:max-w-3xl bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm outline-none text-foreground rounded-md p-3 text-sm';
        ta.value = currentStr;
        ta.addEventListener('input', () => updateConfig({ [field.key]: ta.value }));
        wrap.appendChild(ta);
      } else {
        const rel = document.createElement('div');
        rel.className = 'relative w-full';
        const inp = document.createElement('input');
        inp.type = field.isPositiveInteger ? 'number' : 'text';
        inp.value = currentStr;
        inp.className = 'w-full bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm outline-none text-foreground rounded-md px-3 py-2 text-sm' + (isCustom ? ' pr-8' : '');
        inp.placeholder = serverDefault != null ? `Default: ${serverDefault}` : '';
        inp.addEventListener('input', () => updateConfig({ [field.key]: inp.value }));
        rel.appendChild(inp);
        if (isCustom) {
          const reset = document.createElement('button');
          reset.type = 'button';
          reset.className =
            'absolute top-1/2 right-2 inline-flex h-5 w-5 -translate-y-1/2 items-center justify-center rounded transition-colors hover:bg-muted';
          reset.title = 'Reset to default';
          reset.textContent = '↺';
          reset.addEventListener('click', () => {
            updateConfig({ [field.key]: '' });
            inp.value = '';
          });
          rel.appendChild(reset);
        }
        wrap.appendChild(rel);
      }
    }

    if (field.help) {
      const p = document.createElement('p');
      p.className = 'mt-1 text-xs text-muted-foreground';
      p.textContent = translate(field.help);
      wrap.appendChild(p);
    }

    root.appendChild(wrap);
  }
  return root;
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
