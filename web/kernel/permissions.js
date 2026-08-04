/**
 * permissions.js — staged "always allow" tool permissions + verification.
 * Staging lives here (settings page stages, Save applies via verify()).
 */
import { store } from './store.js';
import { log } from './logger.js';

export const ALWAYS_ALLOWED_KEY = 'LlamaUi.alwaysAllowedTools';

export const permissionsStore = store(new Set());

export function loadPermissions() {
  try {
    const raw = localStorage.getItem(ALWAYS_ALLOWED_KEY);
    const parsed = raw ? JSON.parse(raw) : [];
    permissionsStore.set(new Set(Array.isArray(parsed) ? parsed : []));
  } catch (err) {
    log.warn('LLMUI-CFG-000', 'permissions: stored set unreadable', String(err));
  }
}

function persist() {
  try {
    localStorage.setItem(ALWAYS_ALLOWED_KEY, JSON.stringify([...permissionsStore.get()]));
  } catch (err) {
    log.error('LLMUI-CFG-001', 'permissions: persist failed', String(err));
  }
}

export function hasTool(key) {
  return permissionsStore.get().has(key);
}

export function allowTools(keys) {
  permissionsStore.update((s) => new Set([...s, ...keys]));
  persist();
}

export function revokeTool(key) {
  permissionsStore.update((s) => {
    const next = new Set(s);
    next.delete(key);
    return next;
  });
  persist();
}

/* ---- staging (Settings Tools tab) ---- */
let staged = null; // null = no staged changes; else Set of desired keys

export function stageState() {
  return staged;
}

export function stageChecked(key) {
  if (staged === null) staged = new Set(permissionsStore.get());
  if (staged.has(key)) staged.delete(key);
  else staged.add(key);
  return staged;
}

export function stageAdditions() {
  if (!staged) return [];
  return [...staged].filter((k) => !permissionsStore.get().has(k));
}

export function stageRemovals() {
  if (!staged) return [];
  return [...permissionsStore.get()].filter((k) => !staged.has(k));
}

export function stageHasPending() {
  return staged !== null;
}

/** Apply staged changes (called after verification passes). */
export function stageApply() {
  if (!staged) return;
  const additions = stageAdditions();
  const removals = stageRemovals();
  if (additions.length) allowTools(additions);
  for (const key of removals) revokeTool(key);
  staged = null;
}

export function stageDiscard() {
  staged = null;
}

/**
 * Verify a risky action via the kernel verification dialog (implemented in
 * web/settings/dialogs by Agent B; registered here as a contract stub).
 */
let verifier = null;
export function registerVerifier(fn) {
  verifier = fn;
}
export async function verify(request) {
  if (!verifier) {
    log.warn('LLMUI-TL-003', 'verification: no dialog registered');
    return false;
  }
  return verifier(request);
}
