import { chromium } from 'playwright';
import { mkdirSync, writeFileSync } from 'fs';
import { join, dirname } from 'path';
import { fileURLToPath } from 'url';

// Capture the CURRENT vanilla UI (dist-web) for the parity-baseline views.
const ROOT = join(dirname(fileURLToPath(import.meta.url)), '..');
const BASE = process.env.BASE_URL || 'http://localhost:8766';
const OUT = process.argv[2] || join(ROOT, 'tests', 'vanilla-current');

const VIEWS = [
  ['chat-empty', '#/'],
  ['chat-new', '#/chat/new'],
  ['mcp-servers', '#/mcp-servers'],
  ['search', '#/search'],
  ['settings-general', '#/settings/general'],
  ['settings-display', '#/settings/display'],
  ['settings-sampling', '#/settings/sampling'],
  ['settings-penalties', '#/settings/penalties'],
  ['settings-tools', '#/settings/tools'],
  ['settings-agentic', '#/settings/agentic'],
  ['settings-developer', '#/settings/developer'],
  ['settings-import-export', '#/settings/import-export']
];

const THEMES = ['dark', 'light', 'system', 'nord', 'amoled', 'cobalt', 'dracula', 'gruvbox', 'gruvbox-light', 'ionized-purple', 'solarized'];

mkdirSync(OUT, { recursive: true });
const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 1400, height: 900 } });

// match the parity-baseline config: dark theme, english
await page.addInitScript(() => {
  localStorage.setItem('lang', 'en');
  localStorage.setItem('mode-watcher-mode', 'dark');
  localStorage.setItem('mode-watcher-theme', 'dark');
  localStorage.setItem('LlamaUi.config', JSON.stringify({ theme: 'light' }));
});

for (const [name, hash] of VIEWS) {
  try {
    await page.goto(`${BASE}/${hash}`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(1500);
    await page.screenshot({ path: join(OUT, `${name}.png`) });
    console.log('captured:', name);
  } catch (e) {
    console.log('FAILED:', name, String(e).slice(0, 80));
  }
}

for (const theme of THEMES) {
  try {
    await page.addInitScript((th) => {
      localStorage.setItem('LlamaUi.config', JSON.stringify({ theme: th }));
      localStorage.setItem('mode-watcher-theme', th);
      localStorage.setItem('mode-watcher-mode', th === 'light' || th === 'gruvbox-light' ? 'light' : 'dark');
    }, theme);
    await page.goto(`${BASE}/?t=${theme}#/settings/general`, { waitUntil: 'networkidle' });
    await page.waitForTimeout(1200);
    await page.screenshot({ path: join(OUT, `theme-${theme}.png`) });
    console.log('captured: theme-' + theme);
  } catch (e) {
    console.log('FAILED theme:', theme, String(e).slice(0, 80));
  }
}
writeFileSync(join(OUT, 'manifest.json'), JSON.stringify({ base: BASE, captured: VIEWS.map((v) => v[0]) }));
await browser.close();
