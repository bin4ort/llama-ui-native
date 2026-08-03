/**
 * Theme class toggle — adds/removes CSS classes on <html>
 * for custom themes. Uses the same approach as mode-watcher's .dark class.
 */

import { setMode } from 'mode-watcher';

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

/**
 * Single entry point for applying a theme. mode-watcher's setMode handles the
 * base light/dark/system modes, applyThemeClass handles the custom class
 * themes. Both must run together or custom themes stay stale until reload.
 */
export function applyTheme(mode: string): void {
	setMode(normalizeThemeForSetMode(mode));
	applyThemeClass(mode);
}
