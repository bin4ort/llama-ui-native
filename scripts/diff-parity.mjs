// diff-parity.mjs — Phase 3: pixel-diff new app vs parity-baseline.
// Usage: node scripts/diff-parity.mjs <newDir>
import { PNG } from 'pngjs';
import fs from 'node:fs';
import path from 'node:path';
const NEW = process.argv[2];
const BASE = path.resolve('parity-baseline');
let worst = [];
for (const f of fs.readdirSync(BASE).filter((f) => f.endsWith('.png'))) {
  const nf = path.join(NEW, f);
  if (!fs.existsSync(nf)) { worst.push([f, 'MISSING']); continue; }
  const a = PNG.sync.read(fs.readFileSync(path.join(BASE, f)));
  const b = PNG.sync.read(fs.readFileSync(nf));
  if (a.width !== b.width || a.height !== b.height) { worst.push([f, `size ${a.width}x${a.height} vs ${b.width}x${b.height}`]); continue; }
  let diff = 0;
  for (let i = 0; i < a.data.length; i += 4) {
    const d = Math.abs(a.data[i] - b.data[i]) + Math.abs(a.data[i + 1] - b.data[i + 1]) + Math.abs(a.data[i + 2] - b.data[i + 2]);
    if (d > 60) diff++;
  }
  const pct = (100 * diff) / (a.width * a.height);
  worst.push([f, `${pct.toFixed(2)}%`]);
}
worst.sort((x, y) => (parseFloat(y[1]) || 0) - (parseFloat(x[1]) || 0));
for (const [f, pct] of worst) console.log(`${pct.padEnd(10)} ${f}`);
