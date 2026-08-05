/**
 * presets-picker.js — Agent B: full preset picker dialog.
 * Registered via kernel presets.registerPicker(); Agent A's chat bar calls
 * presets.openPicker(). Resolves with the picked preset (or null for
 * "Default" / dismissal). Includes the wizard shortcut.
 */
import { t, showModal, log, presets } from '../kernel/index.js';
import { openWizardDialog } from './presets-wizard.js';

export function registerPresetPicker() {
  presets.registerPicker(() =>
    new Promise((resolve) => {
      const body = document.createElement('div');
      body.className = 'space-y-2';

      const search = document.createElement('input');
      search.className = 'h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
      search.placeholder = t('Search presets...');
      body.appendChild(search);

      const list = document.createElement('div');
      list.className = 'max-h-72 space-y-1 overflow-y-auto';
      body.appendChild(list);

      const renderList = () => {
        const q = search.value.trim().toLowerCase();
        const all = presets.getPresets();
        const items = all
          .filter(
            (p) =>
              !q ||
              p.name.toLowerCase().includes(q) ||
              (p.description ?? '').toLowerCase().includes(q)
          )
          .sort((a, b) => (b.favorite ? 1 : 0) - (a.favorite ? 1 : 0) || a.name.localeCompare(b.name));

        list.replaceChildren();
        if (items.length === 0) {
          const p = document.createElement('p');
          p.className = 'py-4 text-center text-sm text-muted-foreground';
          p.textContent = t('No matches');
          list.appendChild(p);
          return;
        }
        for (const preset of items) {
          const row = document.createElement('button');
          row.type = 'button';
          row.className =
            'flex w-full items-center gap-2 rounded-md border border-border/40 bg-background px-3 py-2 text-left hover:bg-accent';
          const star = document.createElement('span');
          star.className = 'shrink-0 text-amber-400';
          star.textContent = preset.favorite ? '★' : '';
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
          row.addEventListener('click', () => {
            resolve(preset);
            handle.close();
          });
          list.appendChild(row);
        }
      };
      search.addEventListener('input', renderList);
      renderList();

      const footer = document.createElement('div');
      footer.className = 'flex items-center justify-between gap-2 pt-2';
      const defaultBtn = document.createElement('button');
      defaultBtn.type = 'button';
      defaultBtn.className = 'h-9 rounded-md border border-input px-3 text-sm hover:bg-accent';
      defaultBtn.textContent = t('Default');
      defaultBtn.title = t('Use the default system prompt (no persona)');
      defaultBtn.addEventListener('click', () => {
        resolve(null);
        handle.close();
      });
      footer.appendChild(defaultBtn);

      const wizardBtn = document.createElement('button');
      wizardBtn.type = 'button';
      wizardBtn.className =
        'h-9 rounded-md bg-primary px-3 text-sm font-medium text-primary-foreground hover:bg-primary/90';
      wizardBtn.textContent = t('Create with wizard…');
      wizardBtn.addEventListener('click', () => {
        openWizardDialog((preset) => {
          resolve(preset);
          handle.close();
        });
      });
      footer.appendChild(wizardBtn);
      body.appendChild(footer);

      const handle = showModal({
        title: t('Choose a prompt preset'),
        content: () => body
      });
    })
  );
  log.info('LLMUI-PRS-005', 'presets: picker registered');
}
