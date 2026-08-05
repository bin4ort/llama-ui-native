/**
 * presets.js — Agent B: prompt preset manager + wizard.
 * Library CRUD via kernel presets-store; the wizard generates a detailed
 * system prompt through kernel api.chatCompletion using the professional
 * meta-prompt (port of PRESET_WIZARD_META_PROMPT from the current app).
 */
import { t, log, presets } from '../kernel/index.js';
import { button, checkboxField } from './fields.js';
import { openWizardDialog } from './presets-wizard.js';


export function renderPresetsPage(container) {
  const root = document.createElement('div');
  root.className = 'mx-auto w-full max-w-3xl p-6';
  const h = document.createElement('h2');
  h.className = 'text-lg font-semibold mb-1';
  h.textContent = t('Prompt presets');
  root.appendChild(h);
  const sub = document.createElement('p');
  sub.className = 'text-sm text-muted-foreground mb-4';
  sub.textContent = t('Reusable system prompts (personas). Apply them from the chat bar or let the model switch via change_preset.');
  root.appendChild(sub);

  const search = document.createElement('input');
  search.placeholder = t('Search presets...');
  search.className = 'mb-4 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
  root.appendChild(search);

  const wizardBtn = button(t('Create with wizard…'), () => openWizardDialog(() => renderList()), 'outline');
  root.appendChild(wizardBtn);

  const list = document.createElement('div');
  list.className = 'mt-4 space-y-2';
  root.appendChild(list);

  const editBuffers = new Map();

  function renderList() {
    const q = search.value.trim().toLowerCase();
    const items = presets.getPresets().filter(
      (p) => !q || p.name.toLowerCase().includes(q) || (p.description ?? '').toLowerCase().includes(q)
    );
    list.replaceChildren();
    if (items.length === 0) {
      const p = document.createElement('p');
      p.className = 'py-6 text-center text-sm text-muted-foreground';
      p.textContent = presets.getPresets().length === 0 ? t('No presets yet') : t('No matches');
      list.appendChild(p);
      return;
    }
    for (const preset of items) {
      const editing = editBuffers.has(preset.id);
      const card = document.createElement('div');
      card.className = 'rounded-md border border-border/40 bg-card p-3';
      if (!editing) {
        const row = document.createElement('div');
        row.className = 'flex items-center gap-2';
        const star = document.createElement('button');
        star.type = 'button';
        star.className = 'shrink-0 p-1 text-muted-foreground hover:text-foreground';
        star.title = preset.favorite ? t('Remove from favorites') : t('Add to favorites');
        star.textContent = preset.favorite ? '★' : '☆';
        star.addEventListener('click', () => {
          presets.toggleFavorite(preset.id);
          renderList();
        });
        row.appendChild(star);
        const text = document.createElement('div');
        text.className = 'min-w-0 flex-1';
        const name = document.createElement('div');
        name.className = 'truncate text-sm font-medium';
        name.textContent = preset.name;
        text.appendChild(name);
        if (preset.description) {
          const d = document.createElement('div');
          d.className = 'truncate text-xs text-muted-foreground';
          d.textContent = preset.description;
          text.appendChild(d);
        }
        row.appendChild(text);
        row.appendChild(button(t('Edit'), () => {
          editBuffers.set(preset.id, { name: preset.name, description: preset.description ?? '', content: preset.content });
          renderList();
        }, 'ghost'));
        row.appendChild(button(t('Delete'), () => {
          presets.removePreset(preset.id);
          renderList();
        }, 'ghost'));
        card.appendChild(row);
      } else {
        const buf = editBuffers.get(preset.id);
        const makeInput = (value, onInput) => {
          const i = document.createElement('input');
          i.className = 'mb-2 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
          i.value = value;
          i.addEventListener('input', () => onInput(i.value));
          return i;
        };
        const nameIn = makeInput(buf.name, (v) => (buf.name = v));
        const descIn = makeInput(buf.description, (v) => (buf.description = v));
        const content = document.createElement('textarea');
        content.className = 'mb-2 w-full rounded-md border border-input bg-background p-2 font-mono text-xs';
        content.rows = 8;
        content.value = buf.content;
        content.addEventListener('input', () => (buf.content = content.value));
        card.appendChild(nameIn);
        card.appendChild(descIn);
        card.appendChild(content);
        const row = document.createElement('div');
        row.className = 'flex items-center gap-2';
        row.appendChild(button(t('Save'), () => {
          presets.updatePreset(preset.id, buf);
          editBuffers.delete(preset.id);
          renderList();
        }));
        row.appendChild(button(t('Cancel'), () => {
          editBuffers.delete(preset.id);
          renderList();
        }, 'outline'));
        card.appendChild(row);
      }
      list.appendChild(card);
    }
  }
  search.addEventListener('input', renderList);
  renderList();

  container.replaceChildren(root);
}

