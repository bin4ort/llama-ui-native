/**
 * kernel/index.js — kernel facade (frozen API surface for app/settings trees).
 */
export { store, derived, bus } from './store.js';
export * as router from './router.js';
export { log, LlmUiError, setLogLevel, getLogLevel, getRingBuffer, getRingEntries, LOG_LEVELS } from './logger.js';
export * as errorCodes from './error-codes.js';
export * as api from './api.js';
export { t, tr, setLang, getLang, SUPPORTED_LANGS } from './i18n.js';
export * as i18n from './i18n.js';
export * as theme from './theme.js';
export * as settings from './settings-store.js';
export {
  configStore, overridesStore, settingsBus, loadConfig, saveConfig, updateConfig,
  resetConfig, config, CONFIG_KEY, OVERRIDES_KEY
} from './settings-store.js';
export * as db from './db.js';
export * as presets from './presets-store.js';
export * as permissions from './permissions.js';
export { showModal, closeModal, modalState, subscribeModal, mountModalHost } from './modal.js';
export { button, checkbox, collapsible } from './ui.js';
export { toast, toastApi, mountToasts } from './toast.js';
