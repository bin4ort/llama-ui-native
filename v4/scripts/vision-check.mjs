#!/usr/bin/env node
/**
 * vision-check.mjs — verify v4 UI screenshots using the local vision model
 * (Qwen2.5-VL on localhost:8080). Sends the PNG + a checklist and prints
 * the model's verdict per item.
 *
 * Usage: node v4/scripts/vision-check.mjs <screenshot.png> "<checklist>"
 */
import { readFileSync } from 'fs';

const IMG = process.argv[2];
const CHECKLIST = process.argv[3] || 'Describe the UI and check: dark theme, rounded sidebar strip on the left with icon buttons, centered greeting text, rounded composer bar with a circular send button at the bottom.';
const BASE = process.env.BACKEND || 'http://localhost:8080';

if (!IMG) {
  console.error('usage: node vision-check.mjs <screenshot.png> "<checklist>"');
  process.exit(1);
}

const b64 = readFileSync(IMG).toString('base64');
const body = {
  model: '',
  messages: [
    {
      role: 'user',
      content: [
        { type: 'text', text: CHECKLIST },
        { type: 'image_url', image_url: { url: `data:image/png;base64,${b64}` } }
      ]
    }
  ],
  temperature: 0.2,
  max_tokens: 700,
  stream: false
};

const res = await fetch(`${BASE}/v1/chat/completions`, {
  method: 'POST',
  headers: { 'Content-Type': 'application/json' },
  body: JSON.stringify(body)
});
if (!res.ok) {
  console.error('vision request failed:', res.status, await res.text().catch(() => ''));
  process.exit(1);
}
const data = await res.json();
const content = data?.choices?.[0]?.message?.content ?? '(no content)';
console.log(content);
