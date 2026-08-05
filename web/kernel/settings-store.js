/**
 * settings-store.js — LlamaUi.config persistence (same key as today),
 * server-prop sync + userOverrides, and the logLevel setting.
 */
import { store, bus } from './store.js';
import { setLogLevel } from './logger.js';

export const CONFIG_KEY = 'LlamaUi.config';
export const OVERRIDES_KEY = 'LlamaUi.userOverrides';

const DEFAULTS = {
  language: 'en',
  theme: 'system',
  systemMessage: '',
  systemPromptPresets: '[]',
  logLevel: 2,
  showSystemMessage: true,
  jsSandboxEnabled: false,
  symbolicMathEnabled: false,
  presetToolsEnabled: false,
  toolCalculateEnabled: true,
  toolFetchUrlEnabled: true,
  toolToTableEnabled: true,
  toolJsonEnabled: true,
  toolClipboardEnabled: true,
  toolNotifyEnabled: true,
  toolTodoEnabled: true,
  toolWeatherEnabled: true,
  toolWikipediaEnabled: true,
  toolPlotChartEnabled: true,
  customJson: '',
  customCss: '',
  apiKey: '',
  serverEndpoint: 'http://localhost:8080',
  webUiAddress: '',
  enableWebUi: false
};

export const configStore = store({ ...DEFAULTS });
export const overridesStore = store(new Set());
export const settingsBus = bus();

export function loadConfig() {
  try {
    const raw = localStorage.getItem(CONFIG_KEY);
    const saved = raw ? JSON.parse(raw) : {};
    const merged = { ...DEFAULTS, ...saved };
    // Legacy theme migration (v0.4.x): the old app stored the theme in
    // mode-watcher keys; the config only ever saw a copy. Honor the live
    // value and persist it back into config so the theme survives the swap.
    if (saved.theme === undefined) {
      const legacyTheme =
        localStorage.getItem('mode-watcher-theme') || localStorage.getItem('mode-watcher-mode');
      if (legacyTheme && legacyTheme !== 'system') {
        merged.theme = legacyTheme;
        localStorage.setItem(CONFIG_KEY, JSON.stringify(merged));
      }
    }
    configStore.set(merged);
    const savedOverrides = JSON.parse(localStorage.getItem(OVERRIDES_KEY) || '[]');
    overridesStore.set(new Set(Array.isArray(savedOverrides) ? savedOverrides : []));
    setLogLevel(merged.logLevel);
    return merged;
  } catch (err) {
    configStore.set({ ...DEFAULTS });
    return configStore.get();
  }
}

export function saveConfig() {
  try {
    localStorage.setItem(CONFIG_KEY, JSON.stringify(configStore.get()));
    localStorage.setItem(OVERRIDES_KEY, JSON.stringify([...overridesStore.get()]));
  } catch {
    /* storage full — settings bus consumers already notified below */
  }
  settingsBus.emit('saved', configStore.get());
}

export function updateConfig(patch) {
  configStore.update((cfg) => ({ ...cfg, ...patch }));
  if ('logLevel' in patch) setLogLevel(patch.logLevel);
  if ('language' in patch) {
    import('./i18n.js').then(({ setLang }) => setLang(patch.language));
  }
  if ('theme' in patch) {
    import('./theme.js').then(({ applyTheme }) => applyTheme(patch.theme));
  }
  saveConfig();
}

export function resetConfig() {
  configStore.set({ ...DEFAULTS });
  overridesStore.set(new Set());
  saveConfig();
}

export const config = () => configStore.get();
