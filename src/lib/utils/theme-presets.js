/**
 * Theme class toggle — adds/removes CSS classes on <html>
 * for custom themes. Uses the same approach as mode-watcher's .dark class.
 */
const DARK_CLASSES = [
    'amoled',
    'nord', 'dracula', 'cobalt', 'solarized', 'gruvbox',
    'ionized-purple', 'ionized-red', 'ionized-cyan'
];
const LIGHT_CLASSES = ['snow', 'gruvbox-light'];
const CUSTOM_CLASSES = [...DARK_CLASSES, ...LIGHT_CLASSES];
export function isCustomTheme(mode) {
    return CUSTOM_CLASSES.includes(mode);
}
export function normalizeThemeForSetMode(mode) {
    if (DARK_CLASSES.includes(mode))
        return 'dark';
    if (LIGHT_CLASSES.includes(mode))
        return 'light';
    return mode;
}
export function applyThemeClass(mode) {
    const root = document.documentElement;
    CUSTOM_CLASSES.forEach((c) => root.classList.remove(c));
    if (isCustomTheme(mode)) {
        root.classList.add(mode);
    }
}
