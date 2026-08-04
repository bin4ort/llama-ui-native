/**
 * theme.js — dark/light/system + custom theme classes on <html>.
 * Ports theme-presets.ts + mode-watcher usage (class toggles only).
 */
export const THEME_CLASSES = {
  dark: [
    'amoled', 'nord', 'dracula', 'cobalt', 'solarized', 'gruvbox',
    'ionized-purple', 'ionized-red', 'ionized-cyan'
  ],
  light: ['snow', 'gruvbox-light']
};

const ALL_CUSTOM = [...THEME_CLASSES.dark, ...THEME_CLASSES.light];

export function isCustomTheme(mode) {
  return ALL_CUSTOM.includes(mode);
}

export function normalizeTheme(mode) {
  if (THEME_CLASSES.dark.includes(mode)) return 'dark';
  if (THEME_CLASSES.light.includes(mode)) return 'light';
  return mode === 'light' || mode === 'dark' || mode === 'system' ? mode : 'system';
}

export function applyThemeClass(mode) {
  const root = document.documentElement;
  for (const c of ALL_CUSTOM) root.classList.remove(c);
  if (isCustomTheme(mode)) root.classList.add(mode);
}

export function applyTheme(mode) {
  const normalized = normalizeTheme(mode);
  rootMode(normalized);
  applyThemeClass(mode);
}

function rootMode(mode) {
  const root = document.documentElement;
  root.classList.toggle('dark', mode === 'dark' || (mode === 'system' && systemDark()));
}

export function systemDark() {
  return window.matchMedia('(prefers-color-scheme: dark)').matches;
}

export function initTheme(savedMode) {
  applyTheme(savedMode || 'system');
  window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', () => {
    const mode = savedMode || 'system';
    if (mode === 'system') rootMode('system');
  });
}
