import fs from 'node:fs';
import path from 'node:path';
const LANGS = {
  zh: 'zh.mjs', ja: 'ja.mjs', ko: 'ko.mjs', pt: 'pt.mjs', it: 'it.mjs', tr: 'tr.mjs', pl: 'pl.mjs'
};
const en = JSON.parse(fs.readFileSync('frontend/v2/lang/en.json', 'utf8'));
const enKeys = Object.keys(en);
for (const [code, file] of Object.entries(LANGS)) {
  const mod = await import('./lang-translations/' + file);
  const t = mod.default;
  const missing = enKeys.filter((k) => !(k in t));
  if (missing.length) {
    console.log(code, 'MISSING KEYS:', missing);
    process.exit(1);
  }
  const out = {};
  for (const k of enKeys) out[k] = t[k];
  fs.writeFileSync(`frontend/v2/lang/${code}.json`, JSON.stringify(out, null, 2) + '\n');
  console.log(code, 'written:', Object.keys(out).length, 'keys');
}
