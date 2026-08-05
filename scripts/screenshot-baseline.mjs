#!/usr/bin/env node
/**
 * Phase 0 — screenshot baseline of the CURRENT (SvelteKit) UI.
 *
 * Captures canonical views (chat, all settings sections, MCP, search,
 * themes, dialogs) with a seeded conversation, for the Phase 3 pixel-diff
 * parity gate of the vanilla rewrite (see REFACTOR-PLAN.md).
 *
 * Usage:  node scripts/screenshot-baseline.mjs [outDir]
 * Needs:  the app running on port 8765 (./launch.sh)
 */
import { chromium } from 'playwright';
import { mkdirSync, writeFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';
import { SEED, SEED_DATA } from './baseline-seed.mjs';

const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const BASE = process.env.BASE_URL || 'http://localhost:8765';
const OUT = process.argv[2] || join(ROOT, 'tests', 'baseline');

const SETTINGS_SECTIONS = [
	'general',
	'display',
	'sampling',
	'penalties',
	'tools',
	'agentic',
	'developer',
	'import-export'
];

const THEMES = ['dark', 'light', 'nord', 'amoled', 'snow'];

mkdirSync(OUT, { recursive: true });

const browser = await chromium.launch();
const context = await browser.newContext({ viewport: { width: 1440, height: 900 } });
const page = await context.newPage();

const shots = [];
async function shot(name) {
	const path = join(OUT, `${name}.png`);
	await page.screenshot({ path, fullPage: false });
	shots.push({ name, path });
	console.log('captured:', name);
}

try {
	// Seeded, default theme
	await page.addInitScript(() => {
		localStorage.setItem('lang', 'en');
		localStorage.setItem('mode-watcher-mode', 'dark');
		localStorage.setItem('mode-watcher-theme', 'dark');
	});
	// Boot without the seed so the app creates its own IndexedDB (exact
	// schema), then seed via a no-version connection, reload, and navigate
	// via hash — loadConversation always runs after the data exists.
	await page.goto(`${BASE}/?native=1#/`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(3000);
	await page.evaluate(() => {
		const cfg = JSON.parse(localStorage.getItem('LlamaUi.config') || '{}');
		cfg.alwaysShowSidebarOnDesktop = true; // show the conversation list (sidebar is a collapsed strip by default)
		localStorage.setItem('LlamaUi.config', JSON.stringify(cfg));
	});
	await page.evaluate(SEED, JSON.stringify(SEED_DATA));
	await page.waitForFunction(() => window.__seeded === true, null, { timeout: 10000 });
	// Reload: applies alwaysShowSidebarOnDesktop at boot (data persists in
	// this context), then navigate via hash.
	await page.reload({ waitUntil: 'networkidle' });
	await page.waitForTimeout(2000);
	await page.goto(`${BASE}/?native=1#/chat/baseline-conv-1`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(2500);
	await shot('01-chat-seeded');

	await page.goto(`${BASE}/?native=1#/`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(1200);
	await shot('02-chat-empty');

	for (const section of SETTINGS_SECTIONS) {
		await page.goto(`${BASE}/?native=1#/settings/${section}`, { waitUntil: 'networkidle' });
		await page.waitForTimeout(900);
		await shot(`03-settings-${section}`);
	}

	await page.goto(`${BASE}/?native=1#/mcp-servers`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(900);
	await shot('04-mcp-servers');

	await page.goto(`${BASE}/?native=1#/search`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(900);
	await shot('05-search');

	// Themes on the general settings page
	for (const theme of THEMES) {
		await page.addInitScript((t) => {
			localStorage.setItem('mode-watcher-mode', t === 'light' ? 'light' : 'dark');
			localStorage.setItem('mode-watcher-theme', t);
		}, theme);
		await page.goto(`${BASE}/?native=1#/settings/general`, { waitUntil: 'networkidle' });
		await page.waitForTimeout(900);
		await shot(`06-theme-${theme}`);
	}

	// Dialogs on the seeded chat
	await page.goto(`${BASE}/?native=1#/chat/baseline-conv-1`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(1500);
	try {
		await page.click('button[title="Choose a prompt preset"]');
		await page.waitForTimeout(700);
		await shot('07-preset-picker');
		try {
			await page.click('text=All presets…');
			await page.waitForTimeout(900);
			await shot('08-preset-picker-full');
		} catch {
			console.log('skipped: preset picker full dialog');
		}
	} catch {
		console.log('skipped: preset quick picker (button not found)');
	}

	writeFileSync(join(OUT, 'manifest.json'), JSON.stringify({ base: BASE, captured: shots, date: new Date().toISOString() }, null, 2));
	console.log('done:', shots.length, 'shots ->', OUT);
} finally {
	await browser.close();
}
