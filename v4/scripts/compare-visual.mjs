#!/usr/bin/env node
/**
 * compare-visual.mjs — iterative vision comparison (single image per
 * request — the local model fails with two images in one prompt):
 *
 *   1. describe the ORIGINAL image exhaustively (sidebar / messages /
 *      composer / footer / colors / spacing / text)
 *   2. detail-drill: for each element listed, exact size, color,
 *      spacing, glyph, alignment
 *   3. describe the REIMPLEMENTATION exhaustively (same prompt)
 *   4. detail-drill for the reimplementation
 *   5. text-only: compare both descriptions, list EVERY difference
 *   6. text-only: concrete fixes to make the reimplementation match
 *
 * Usage: node v4/scripts/compare-visual.mjs <v2.jpg> <v4.jpg> [label]
 * Env:  BACKEND (default http://127.0.0.1:8081)
 */
import { readFileSync, writeFileSync } from 'fs';

const [v2Path, v4Path, label = 'page'] = process.argv.slice(2);
const BASE = process.env.BACKEND || 'http://127.0.0.1:8081';
const OUT = `/tmp/opencode/compare-${label.replace(/\s+/g, '-')}.txt`;

if (!v2Path || !v4Path) {
  console.error('usage: compare-visual.mjs <v2.jpg> <v4.jpg> [label]');
  process.exit(1);
}

const img = (p) => `data:image/${p.endsWith('.png') ? 'png' : 'jpeg'};base64,${readFileSync(p).toString('base64')}`;

async function ask(messages, maxTokens = 800) {
  const body = {
    model: '',
    messages,
    stream: false,
    max_tokens: maxTokens,
    temperature: 0.2,
  };
  const r = await fetch(`${BASE}/v1/chat/completions`, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(body),
  });
  if (!r.ok) throw new Error(`HTTP ${r.status}: ${(await r.text()).slice(0, 200)}`);
  return (await r.json()).choices[0].message.content;
}

const DESCRIBE = `This is a screenshot of a desktop chat application. Describe it EXHAUSTIVELY, section by section:
1. Left sidebar: exact width, background color, every icon/button (position, order, glyph, size, tooltips if visible), any text, any footer element.
2. Main area: greeting or messages (exact text, font size, alignment), spacing, any cards or bubbles (colors, radii).
3. Bottom composer bar: every single element left to right (buttons, icons, input, labels), shapes, colors, sizes, borders.
4. Colors: describe the exact palette (background, surfaces, text, accent).
5. Anything else visible: dialogs, toasts, status indicators, version strings.`;

async function main() {
  let log = `=== compare-visual: ${label} ===\n(v2: ${v2Path}, v4: ${v4Path})\n\n`;
  let out = '';
  console.error(`[compare] ${label}: describe original`);
  const r1 = await ask([{ role: 'user', content: [{ type: 'text', text: DESCRIBE }, { type: 'image_url', image_url: { url: img(v2Path) } }] }]);
  log += `--- ORIGINAL (image 1) ---\n${r1}\n\n`;
  const r1d = await ask([{ role: 'user', content: [{ type: 'text', text: DESCRIBE }, { type: 'image_url', image_url: { url: img(v2Path) } }] }, { role: 'assistant', content: r1 }, { role: 'user', content: `Now drill into details: for every element you listed give exact specifics — pixel-ish sizes, hex colors, spacing, icon glyphs, button shapes, alignment. If you missed the composer bar or footer, describe them now.` }]);
  log += `--- ORIGINAL details ---\n${r1d}\n\n`;

  console.error(`[compare] ${label}: describe reimplementation`);
  const r2 = await ask([{ role: 'user', content: [{ type: 'text', text: DESCRIBE }, { type: 'image_url', image_url: { url: img(v4Path) } }] }]);
  log += `--- REIMPLEMENTATION (image 2) ---\n${r2}\n\n`;
  const r2d = await ask([{ role: 'user', content: [{ type: 'text', text: DESCRIBE }, { type: 'image_url', image_url: { url: img(v4Path) } }] }, { role: 'assistant', content: r2 }, { role: 'user', content: `Now drill into details: for every element you listed give exact specifics — sizes, hex colors, spacing, icon glyphs, button shapes, alignment. If you missed the composer bar or footer, describe them now.` }]);
  log += `--- REIMPLEMENTATION details ---\n${r2d}\n\n`;

  console.error(`[compare] ${label}: compare`);
  const r3 = await ask([{ role: 'user', content: `You compared two screenshots of the same chat app. Here are the descriptions:

=== ORIGINAL ===
${r1}
${r1d}

=== REIMPLEMENTATION ===
${r2}
${r2d}

List EVERY difference between the original and the reimplementation. For each: WHERE (sidebar/composer/messages/footer/other), WHAT differs (exact element, color, size, spacing, icon, text, presence/absence), and which side has it. Be exhaustive — order of buttons, glyphs, shapes, padding, alignment, strings.` }]);
  log += `--- DIFFERENCES ---\n${r3}\n\n`;

  console.error(`[compare] ${label}: fixes`);
  const r4 = await ask([{ role: 'user', content: `${r3}\n\nFor each difference, give a CONCRETE fix to make the reimplementation match the original exactly: component name, exact styling (color, radius, size, padding), exact text/icon. Number the fixes.` }]);
  log += `--- FIXES ---\n${r4}\n\n`;

  writeFileSync(OUT, log);
  console.log(log);
  console.error(`[compare] saved to ${OUT}`);
}

main().catch((e) => { console.error(e); process.exit(1); });
