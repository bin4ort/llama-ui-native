/**
 * general.js — Agent B: General settings section.
 * Language (12 langs), theme (system/dark/light + 11 custom palettes),
 * model selection (server list + favorites), default system message.
 */
import { t, i18n, theme, updateConfig, config } from '../../kernel/index.js';
import { selectField, textField, sectionCard, button, row } from '../fields.js';
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
  ...Object.entries(theme.THEME_CLASSES).flatMap(([mode, names]) =>
    names.map((n) => [n, n])
  )
];

export function renderGeneralSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  // Language
  root.appendChild(sectionCard(t('Language')));
  root.lastChild.appendChild(
    selectField(
      t('Language'),
      config().language ?? 'en',
      i18n.SUPPORTED_LANGS.map((code) => [code, LANG_NAMES[code] ?? code]),
      t('UI language for the whole application.'),
      (v) => updateConfig({ language: v })
    )
  );

  // Theme
  root.appendChild(sectionCard(t('Theme')));
  root.lastChild.appendChild(
    selectField(
      t('Theme'),
      config().theme ?? 'system',
      THEME_OPTIONS,
      t('Custom palettes override the base dark/light mode.'),
      (v) => updateConfig({ theme: v })
    )
  );

  // Model
  root.appendChild(sectionCard(t('Model')));
  const modelCard = root.lastChild;
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
    sel.className = 'h-9 rounded-md border border-input bg-background px-3 text-sm w-full';
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
      const favRow = row(
        t('Favorite models'),
        null,
        t('Star models to pin them to the top of the list.')
      );
      for (const m of ordered.slice(0, 6)) {
        const star = button(
          favorites.has(m.id) ? '★ ' + m.id : m.id,
          () => {
            toggleFavorite(m.id);
            refreshModelList();
          },
          favorites.has(m.id) ? 'default' : 'ghost'
        );
        favRow.appendChild(star);
      }
      modelCard.appendChild(favRow);
    }
  };
  loadFavorites();
  refreshModelList();

  // System message
  root.appendChild(sectionCard(t('Default system prompt')));
  const sys = document.createElement('textarea');
  sys.className = 'mt-1 w-full rounded-md border border-input bg-background p-2 text-sm';
  sys.rows = 4;
  sys.value = config().systemMessage ?? '';
  sys.addEventListener('input', () => updateConfig({ systemMessage: sys.value }));
  root.lastChild.appendChild(sys);

  return root;
}
