#!/usr/bin/env node
/**
 * Phase 0/3 — verify the seeded baseline conversation renders correctly.
 *
 * Uses a PERSISTENT browser profile: boot once, seed the baseline
 * conversation (scripts/baseline-seed.mjs) via evaluate, close; reopen the
 * same profile so the app boots with the data already present (sidebar +
 * messages). Asserts key elements; exits non-zero on failure.
 *
 * Usage:  node scripts/verify-baseline.mjs [baseUrl]
 */
import { chromium } from 'playwright';
import { mkdtempSync, rmSync } from 'fs';
import { tmpdir } from 'os';
import { join } from 'path';
import { SEED, SEED_DATA } from './baseline-seed.mjs';

const BASE = process.argv[2] || process.env.BASE_URL || 'http://localhost:8765';
const PROFILE = process.env.BASELINE_PROFILE || mkdtempSync(join(tmpdir(), 'llamaui-baseline-'));

// 1) Seed phase: boot, wait for the app to settle, write the data.
{
	const browser = await chromium.launchPersistentContext(PROFILE, {
		viewport: { width: 1440, height: 900 }
	});
	const page = browser.pages()[0] ?? (await browser.newPage());
	await page.goto(`${BASE}/?native=1#/`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(3000);
	// Expand the desktop sidebar (collapsed icon strip by default) and keep
	// it expanded: the conversation list lives inside the expanded strip.
	await page.evaluate(() => {
		const cfg = JSON.parse(localStorage.getItem('LlamaUi.config') || '{}');
		cfg.alwaysShowSidebarOnDesktop = true;
		localStorage.setItem('LlamaUi.config', JSON.stringify(cfg));
	});
	await page.evaluate(SEED, JSON.stringify(SEED_DATA));
	await page.waitForFunction(() => window.__seeded === true, null, { timeout: 10000 });
	await browser.close();
}

// 2) Verify phase: reopen the same profile — data exists before boot.
const browser = await chromium.launchPersistentContext(PROFILE, {
	viewport: { width: 1440, height: 900 }
});
let failed = false;
try {
	const page = browser.pages()[0] ?? (await browser.newPage());
	await page.goto(`${BASE}/?native=1#/chat/baseline-conv-1`, { waitUntil: 'networkidle' });
	await page.waitForTimeout(2500);

	const checks = {
		'sidebar conversation name': "text=Baseline conversation",
		'user message bubble': "text=Show me a formatted markdown sample",
		'markdown h2': 'h2:has-text("Markdown sample")',
		'code block': 'pre code, .code-block',
		'markdown table': 'table',
		'list items': 'text=item two'
	};
	for (const [name, sel] of Object.entries(checks)) {
		const n = await page.locator(sel).count();
		const ok = n > 0;
		console.log(`${ok ? 'PASS' : 'FAIL'}  ${name} (${n})`);
		if (!ok) failed = true;
	}
} finally {
	await browser.close();
	if (process.env.BASELINE_PROFILE) rmSync(PROFILE, { recursive: true, force: true });
}
process.exit(failed ? 1 : 0);
