// parity-screenshots.mjs — Phase 0 screenshot baseline of the current UI.
// Run against a running server (the native app on :8765 or a static server
// serving frontend/v2). Output: parity-baseline/<name>.png
//
// Usage: node scripts/parity-screenshots.mjs [baseUrl]
import { chromium } from 'playwright';
import fs from 'node:fs';
import path from 'node:path';

const BASE = process.argv[2] ?? 'http://localhost:8765';
const OUT = path.resolve('parity-baseline');

const THEMES = [
  ['system', 'system'],
  ['light', 'light'],
  ['dark', 'dark'],
  ['amoled', 'amoled'],
  ['nord', 'nord'],
  ['dracula', 'dracula'],
  ['cobalt', 'cobalt'],
  ['solarized', 'solarized-dark'],
  ['gruvbox', 'gruvbox-dark'],
  ['gruvbox-light', 'gruvbox-light'],
  ['ionized', 'ionized-purple']
];

const VIEWS = [
  { name: 'chat-empty', url: '#/' },
  { name: 'chat-new', url: '#/chat/new' },
  { name: 'search', url: '#/search' },
  { name: 'mcp-servers', url: '#/mcp-servers' },
  ...['general', 'display', 'sampling', 'penalties', 'tools', 'agentic', 'developer', 'import-export'].map(
    (s) => ({ name: `settings-${s}`, url: `#/settings/${s}` })
  )
];

fs.mkdirSync(OUT, { recursive: true });

const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 1400, height: 900 } });
const failures = [];

for (const view of VIEWS) {
  try {
    await page.goto(`${BASE}/${view.url}`, { waitUntil: 'domcontentloaded', timeout: 30000 });
    await page.waitForTimeout(1500);
    const file = path.join(OUT, `${view.name}.png`);
    await page.screenshot({ path: file, fullPage: false });
    console.log('OK', view.name, '->', file);
  } catch (err) {
    failures.push(view.name);
    console.log('FAIL', view.name, err.message.split('\n')[0]);
  }
}

// theme sweep on the General settings page (theme dropdown must reflect the config)
for (const [theme, value] of THEMES) {
  try {
    await page.evaluate(
      ([v]) => {
        const cfg = JSON.parse(localStorage.getItem('LlamaUi.config') || '{}');
        cfg.theme = v;
        localStorage.setItem('LlamaUi.config', JSON.stringify(cfg));
      },
      [value]
    );
    await page.goto(`${BASE}/#/settings/general`, { waitUntil: 'domcontentloaded', timeout: 30000 });
    await page.waitForTimeout(1200);
    await page.screenshot({ path: path.join(OUT, `theme-${theme}.png`) });
    console.log('OK theme', theme);
  } catch (err) {
    failures.push(`theme-${theme}`);
    console.log('FAIL theme', theme, err.message.split('\n')[0]);
  }
}

await browser.close();
console.log(failures.length ? `FAILED: ${failures.join(', ')}` : 'All baseline views captured.');
