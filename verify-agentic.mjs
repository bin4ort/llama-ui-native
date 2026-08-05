// verify-agentic.mjs — self-contained: mock SSE server + playwright test.
import { chromium } from 'playwright';
import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';

const ROOT = path.resolve('dist-web');
const MIME = { '.html': 'text/html', '.js': 'text/javascript', '.css': 'text/css', '.svg': 'image/svg+xml', '.woff2': 'font/woff2' };
let requestCount = 0;

const server = http.createServer((req, res) => {
  const url = new URL(req.url, 'http://x');
  if (url.pathname === '/v1/chat/completions' && req.method === 'POST') {
    let body = '';
    req.on('data', (c) => (body += c));
    req.on('end', () => {
      requestCount++;
      const parsed = JSON.parse(body || '{}');
      const msgs = parsed.messages ?? [];
      const lastUser = [...msgs].reverse().find((m) => m.role === 'user')?.content ?? '';
      const hasToolResult = msgs.some((m) => m.role === 'tool');
      res.writeHead(200, { 'Content-Type': 'text/event-stream' });
      if (lastUser.includes('TOOLTEST') && !hasToolResult) {
        res.write(`data: ${JSON.stringify({ choices: [{ delta: { tool_calls: [{ index: 0, id: 'call_1', type: 'function', function: { name: 'calculate', arguments: '{"expression":"2+2"}' } }] } }] })}\n\n`);
        res.write('data: [DONE]\n\n');
        res.end();
        return;
      }
      const reply = lastUser.includes('TOOLTEST')
        ? 'The calculator returned 4. Tool execution worked end to end.'
        : `Mock response to: ${lastUser} — streamed.`;
      const words = reply.split(' ');
      let i = 0;
      const timer = setInterval(() => {
        if (i >= words.length) { clearInterval(timer); res.write('data: [DONE]\n\n'); res.end(); return; }
        res.write(`data: ${JSON.stringify({ choices: [{ delta: { content: words[i] + ' ' } }] })}\n\n`);
        i++;
      }, 10);
    });
    return;
  }
  const file = path.join(ROOT, url.pathname === '/' ? 'index.html' : url.pathname);
  fs.readFile(file, (err, data) => {
    if (err) { res.writeHead(404); res.end(); return; }
    res.writeHead(200, { 'Content-Type': MIME[path.extname(file)] ?? 'application/octet-stream' });
    res.end(data);
  });
});

await new Promise((r) => server.listen(8766, r));
console.log('mock up on :8766');

const browser = await chromium.launch();
const page = await browser.newPage({ viewport: { width: 1400, height: 900 } });
const logs = [];
page.on('console', (m) => logs.push(`[${m.type()}] ${m.text().slice(0, 160)}`));
page.on('pageerror', (e) => logs.push('[ERR] ' + e.message));
await page.goto('http://localhost:8766/?autoapprove=1', { waitUntil: 'domcontentloaded', timeout: 30000 });
await page.evaluate(() => {
  const cfg = JSON.parse(localStorage.getItem('LlamaUi.config') || '{}');
  cfg.serverEndpoint = 'http://localhost:8766';
  localStorage.setItem('LlamaUi.config', JSON.stringify(cfg));
});
await page.reload({ waitUntil: 'networkidle', timeout: 30000 });
await page.waitForTimeout(600);
await page.fill('textarea', 'TOOLTEST');
await page.click('[data-role="send"]');
await page.waitForTimeout(3500);
const chatText = await page.evaluate(() => document.querySelector('[data-role="messages"]').innerText);
console.log('requests:', requestCount);
console.log('tool row rendered:', chatText.includes('calculate'));
console.log('final answer:', chatText.includes('The calculator returned 4'));
console.log('logs:', logs.slice(0, 8).join(' | '));
await browser.close();
server.close();
