/**
 * models.js — Agent B: models store + fetch. List from /v1/models, favorites
 * (LlamaUi.favoriteModels, same key as the current app), active selection
 * persisted in config.model.
 */
import { store } from '../kernel/index.js';
import { api, log } from '../kernel/index.js';

const FAVORITES_KEY = 'LlamaUi.favoriteModels';

export const modelsStore = store({ loaded: false, list: [], error: null });
export const favoriteModelsStore = store(new Set());

export function loadFavorites() {
  try {
    const raw = localStorage.getItem(FAVORITES_KEY);
    favoriteModelsStore.set(new Set(raw ? JSON.parse(raw) : []));
  } catch (err) {
    log.warn('LLMUI-API-002', 'models: favorites unreadable', String(err));
  }
}

export function toggleFavorite(id) {
  favoriteModelsStore.update((s) => {
    const next = new Set(s);
    if (next.has(id)) next.delete(id);
    else next.add(id);
    return next;
  });
  try {
    localStorage.setItem(FAVORITES_KEY, JSON.stringify([...favoriteModelsStore.get()]));
  } catch (err) {
    log.error('LLMUI-CFG-001', 'models: favorites persist failed', String(err));
  }
}

export async function fetchModels() {
  modelsStore.update((s) => ({ ...s, loaded: false }));
  try {
    const list = await api.getModels();
    modelsStore.set({ loaded: true, list, error: null });
    return list;
  } catch (err) {
    modelsStore.set({ loaded: true, list: [], error: err?.code ?? 'LLMUI-API-003' });
    log.error(err?.code ?? 'LLMUI-API-003', 'models: fetch failed', err?.message ?? String(err));
    return [];
  }
}
