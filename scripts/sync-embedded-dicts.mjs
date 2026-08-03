// sync-embedded-dicts.mjs — regenerate the inline DE/RU/EN/ES/FR dicts in
// src/lib/stores/i18n.svelte.ts from frontend/v2/lang/*.json
//
// The lang/*.json files are the source of truth. Run this after editing them:
//   node scripts/sync-embedded-dicts.mjs && npm run build
import fs from 'node:fs';
import path from 'node:path';

const ROOT = path.resolve(import.meta.dirname, '..');
const I18N = path.join(ROOT, 'src/lib/stores/i18n.svelte.ts');
const LANGS = [
  ['DE_FULL', 'de.json'],
  ['RU_FULL', 'ru.json'],
  ['EN_FULL', 'en.json'],
  ['ES_FULL', 'es.json'],
  ['FR_FULL', 'fr.json'],
  ['ZH_FULL', 'zh.json'],
  ['JA_FULL', 'ja.json'],
  ['KO_FULL', 'ko.json'],
  ['PT_FULL', 'pt.json'],
  ['IT_FULL', 'it.json'],
  ['TR_FULL', 'tr.json'],
  ['PL_FULL', 'pl.json'],
];

let src = fs.readFileSync(I18N, 'utf8');

const start = src.indexOf('const DE_FULL');
const end = src.indexOf('function applyDict');
if (start < 0 || end < 0) {
  console.error('Could not locate embedded dicts in i18n.svelte.ts');
  process.exit(1);
}

const blocks = [];
for (const [name, file] of LANGS) {
  const p = path.join(ROOT, 'frontend/v2/lang', file);
  const dict = JSON.parse(fs.readFileSync(p, 'utf8'));
  const lines = [`const ${name}: Record<string,string> = {`];
  for (const [k, v] of Object.entries(dict)) {
    lines.push(`  ${JSON.stringify(k)}: ${JSON.stringify(v)},`);
  }
  lines.push('};', '');
  blocks.push(lines.join('\n'));
  console.log(`${name} <- lang/${file} (${Object.keys(dict).length} keys)`);
}

src = src.slice(0, start) + blocks.join('\n') + src.slice(end);
fs.writeFileSync(I18N, src);
console.log('i18n.svelte.ts updated');
