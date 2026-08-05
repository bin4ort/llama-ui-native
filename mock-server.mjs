// mock-server.mjs — static server for dist-web + mock /v1/chat/completions SSE.
import http from 'node:http';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const ROOT = path.resolve(path.dirname(fileURLToPath(import.meta.url)), 'dist-web');
const PORT = 8766;
const MIME = { '.html': 'text/html', '.js': 'text/javascript', '.css': 'text/css', '.png': 'image/png', '.svg': 'image/svg+xml', '.ico': 'image/x-icon', '.woff2': 'font/woff2', '.json': 'application/json' };

const server = http.createServer((req, res) => {
  const url = new URL(req.url, `http://${req.headers.host}`);
  if (url.pathname === '/v1/chat/completions' && req.method === 'POST') {
    let body = '';
    req.on('data', (c) => (body += c));
    req.on('end', () => {
      console.log('MOCK request:', body.slice(0, 160).replace(/\n/g, ' '));
      const parsed = JSON.parse(body || '{}');
      const msgs = parsed.messages ?? [];
      const lastUser = [...msgs].reverse().find((m) => m.role === 'user')?.content ?? '';
      const hasToolResult = msgs.some((m) => m.role === 'tool');
      let reply;
      if (lastUser.includes('TOOLTEST') && !hasToolResult) {
        // first turn: ask for a tool call
        res.writeHead(200, { 'Content-Type': 'text/event-stream', 'Cache-Control': 'no-cache' });
        res.write('data: ' + JSON.stringify({ choices: [{ delta: { tool_calls: [{ index: 0, id: 'call_1', type: 'function', function: { name: 'calculate', arguments: '{"expression":"2+2"}' } }] } }] }) + '\n\n');
        res.write('data: [DONE]\n\n');
        res.end();
        return;
      }
      if (lastUser.includes('TOOLTEST') && hasToolResult) {
        reply = 'The calculator returned 4. Tool execution worked end to end.';
      } else {
        reply = `Mock response to: ${lastUser} — streamed by the mock server.`;
      }
      res.writeHead(200, { 'Content-Type': 'text/event-stream', 'Cache-Control': 'no-cache' });
      const words = reply.split(' ');
      let i = 0;
      const timer = setInterval(() => {
        if (i >= words.length) {
          clearInterval(timer);
          res.write('data: [DONE]\n\n');
          res.end();
          return;
        }
        const chunk = words[i] + ' ';
        res.write(`data: ${JSON.stringify({ choices: [{ delta: { content: chunk } }] })}\n\n`);
        i++;
      }, 15);
    });
    return;
  }

  if (url.pathname === '/models') {
    res.writeHead(200, { 'Content-Type': 'application/json' });
    res.end(JSON.stringify({ data: [{ id: 'mock-model' }] }));
    return;
  }

  let filePath = path.join(ROOT, url.pathname === '/' ? 'index.html' : url.pathname);
  if (!filePath.startsWith(ROOT)) {
    res.writeHead(403);
    res.end();
    return;
  }
  fs.readFile(filePath, (err, data) => {
    if (err) {
      res.writeHead(404);
      res.end('not found');
      return;
    }
    res.writeHead(200, { 'Content-Type': MIME[path.extname(filePath)] ?? 'application/octet-stream' });
    res.end(data);
  });
});

server.listen(PORT, () => console.log(`mock server on :${PORT}`));
