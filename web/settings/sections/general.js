/**
 * general.js — Agent B: General settings section.
 * Registry-driven (all 16 fields incl. title-generation radio) + the model
 * selector as a special block (server list + favorites).
 */
import { t, theme, updateConfig, config } from '../../kernel/index.js';
import { registrySection } from '../fields.js';
import { FIELDS } from '../registry.js';
import { modelsStore, fetchModels, favoriteModelsStore, loadFavorites, toggleFavorite } from '../models.js';

const LANG_NAMES = {
  de: 'Deutsch', en: 'English', es: 'Español', fr: 'Français', it: 'Italiano',
  ja: '日本語', ko: '한국어', pl: 'Polski', pt: 'Português', ru: 'Русский',
  tr: 'Türkçe', zh: '中文'
};

const THEME_OPTIONS = [
  ['system', 'System'],
  ['dark', 'Dark'],
  ['light', 'Light'],
  ...Object.entries(theme.THEME_CLASSES).flatMap(([mode, names]) => names.map((n) => [n, n]))
];

const SELECT_OPTIONS = {
  language: Object.keys(LANG_NAMES).map((code) => [code, LANG_NAMES[code]]),
  theme: THEME_OPTIONS
};

export function renderGeneralSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  // Model selector (special block, not in the registry)
  const modelCard = document.createElement('div');
  modelCard.className = 'space-y-6';
  const h = document.createElement('h3');
  h.className = 'mb-6 text-base font-semibold';
  h.textContent = t('Model');
  modelCard.appendChild(h);
  const modelStatus = document.createElement('p');
  modelStatus.className = 'text-xs text-muted-foreground';
  modelCard.appendChild(modelStatus);

  const refreshModelList = async () => {
    const list = await fetchModels();
    const state = modelsStore.get();
    if (state.error) {
      modelStatus.textContent = `${t('Could not fetch models')} (${state.error})`;
      return;
    }
    modelStatus.textContent = list.length ? `${list.length} models` : t('No models found');
    const active = config().model ?? '';
    const favorites = favoriteModelsStore.get();
    const ordered = [...list].sort((a, b) => {
      const fa = favorites.has(a.id) ? 0 : 1;
      const fb = favorites.has(b.id) ? 0 : 1;
      return fa - fb || String(a.id).localeCompare(String(b.id));
    });
    const prev = modelCard.querySelector('[data-model-select]');
    if (prev) prev.remove();
    const sel = document.createElement('select');
    sel.dataset.modelSelect = '';
    sel.className = 'w-full bg-muted/60 dark:bg-muted/75 border border-border/30 shadow-sm outline-none text-foreground rounded-md px-3 py-2 text-sm';
    const empty = document.createElement('option');
    empty.value = '';
    empty.textContent = t('No model selected');
    sel.appendChild(empty);
    for (const m of ordered) {
      const o = document.createElement('option');
      o.value = m.id;
      o.textContent = favorites.has(m.id) ? `★ ${m.id}` : m.id;
      o.selected = m.id === active;
      sel.appendChild(o);
    }
    sel.addEventListener('change', () => updateConfig({ model: sel.value || undefined }));
    modelCard.appendChild(sel);
    if (ordered.length) {
      const favRow = document.createElement('div');
      favRow.className = 'flex flex-wrap items-center gap-1.5 pt-2';
      for (const m of ordered.slice(0, 6)) {
        const star = document.createElement('button');
        star.type = 'button';
        star.className =
          'inline-flex items-center gap-1 rounded-md border border-border/30 px-2 py-1 text-xs ' +
          (favorites.has(m.id) ? 'text-amber-500 hover:bg-accent' : 'text-muted-foreground hover:bg-accent hover:text-foreground');
        star.textContent = (favorites.has(m.id) ? '★ ' : '') + m.id;
        star.addEventListener('click', () => {
          toggleFavorite(m.id);
          refreshModelList();
        });
        favRow.appendChild(star);
      }
      modelCard.appendChild(favRow);
    }
  };
  loadFavorites();
  refreshModelList();
  root.appendChild(modelCard);

  // Registry fields (old order: language … maxImageMPixels)
  const fields = registrySection(FIELDS.general, {
    config,
    updateConfig,
    translate: (s) => t(s),
    selectOptions: SELECT_OPTIONS
  });
  root.appendChild(fields);

  return root;
}
