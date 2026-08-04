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
const d = await page.evaluate(() => {
  const el = document.querySelector('[role="dialog"]');
  return { sw: el.scrollWidth, cw: el.clientWidth, w: Math.round(el.getBoundingClientRect().width) };
});
console.log('dialog:', JSON.stringify(d), d.sw <= d.cw ? '=> FITS' : '=> STILL OVERFLOWS');
await browser.close();
