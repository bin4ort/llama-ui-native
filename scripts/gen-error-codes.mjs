// gen-error-codes.mjs — generate docs/ERROR-CODES.md from the frontend
// registry (web/kernel/error-codes.js) + the C registry (web/kernel/error-codes.h).
import fs from 'node:fs';
import { toMarkdown } from '../web/kernel/error-codes.js';

const cHeader = fs.readFileSync('web/kernel/error-codes.h', 'utf8');
const cCodes = [...cHeader.matchAll(/LLMUI_SRV_\d+/g)].map((m) => m[0]);
const cLines = cCodes.map((c) => {
  const num = c.replace('LLMUI_SRV_', '');
  const comment = cHeader.match(new RegExp(`${c}[^\\n]*/\\* ([^*]+) \\*/`))?.[1]?.trim() ?? '';
  return `| LLMUI-SRV-${num} | SRV | ${comment} | active |`;
});

const doc = `${toMarkdown()}\n## Native C server (SRV area)\n\n| Code | Area | Message | Status |\n|---|---|---|---|\n${cLines.join('\n')}\n`;
fs.writeFileSync('docs/ERROR-CODES.md', doc);
console.log('docs/ERROR-CODES.md written');
