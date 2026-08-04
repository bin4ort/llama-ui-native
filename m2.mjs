import { chromium } from 'playwright';
const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 1400, height: 900 } });
await page.goto('http://localhost:8765/', { waitUntil: 'domcontentloaded', timeout: 30000 });
await page.evaluate(() => {
  const cfg = JSON.parse(localStorage.getItem('LlamaUi.config') || '{}');
  cfg.systemPromptPresets = JSON.stringify([
    { id: 'p1', name: 'A very long expert system prompt preset name that keeps going and going', description: 'An even longer one-line description of what this preset does with lots of extra words to test truncation behavior in the picker list', content: 'You are an expert.', favorite: true }
  ]);
  localStorage.setItem('LlamaUi.config', JSON.stringify(cfg));
});
await page.reload({ waitUntil: 'networkidle' });
await page.waitForTimeout(1200);
await page.locator('button[title="Choose a prompt preset"]').first().click();
await page.waitForTimeout(400);
await page.getByText('All presets…').first().click();
await page.waitForTimeout(500);
const r = await page.evaluate(() => {
  const el = document.querySelector('[role="dialog"]');
  const d = el.getBoundingClientRect();
  const out = [];
  for (const n of el.querySelectorAll('*')) {
    const b = n.getBoundingClientRect();
    if (b.width > 0 && (b.right > d.right + 1)) out.push({ tag: n.tagName, cls: (n.className||'').toString().slice(0,70), right: Math.round(b.right), dRight: Math.round(d.right) });
  }
  return { sw: el.scrollWidth, cw: el.clientWidth, offenders: out.slice(0, 8) };
});
console.log(JSON.stringify(r, null, 1));
await browser.close();
