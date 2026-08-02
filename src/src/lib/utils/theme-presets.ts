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

export function isCustomTheme(mode: string): boolean {
	return CUSTOM_CLASSES.includes(mode);
}

export function normalizeThemeForSetMode(mode: string): 'dark' | 'light' | 'system' {
	if (DARK_CLASSES.includes(mode)) return 'dark';
	if (LIGHT_CLASSES.includes(mode)) return 'light';
	return mode as 'dark' | 'light' | 'system';
}

export function applyThemeClass(mode: string): void {
	const root = document.documentElement;
	CUSTOM_CLASSES.forEach((c) => root.classList.remove(c));
	if (isCustomTheme(mode)) {
		root.classList.add(mode);
	}
}
