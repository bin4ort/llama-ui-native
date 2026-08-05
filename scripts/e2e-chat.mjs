import { chromium } from 'playwright';

const b = await chromium.launch();
const p = await b.newPage({ viewport: { width: 1440, height: 900 } });
const logs = [];
p.on('console', (m) => {
	if (m.type() === 'error') logs.push(m.text().slice(0, 160));
});
p.on('pageerror', (e) => logs.push('PAGEERR: ' + String(e).slice(0, 160)));

await p.goto('http://localhost:8765/?test=1#/', { waitUntil: 'networkidle' });
await p.waitForTimeout(1500);

// Send a real message through the composer
await p.fill('textarea', 'Write a short poem about the sea, 4 lines.');
await p.click('button:has-text("Send"), button[type="submit"]');
await p.waitForTimeout(20000); // stream + generation

const state = await p.evaluate(() => {
	const messages = [...document.querySelectorAll('[class*=message]')];
	const bodyText = document.body.innerText;
	return {
		hasUserMsg: bodyText.includes('poem about the sea'),
		hasAssistantText: /sea|wave|ocean/i.test(
			[...document.querySelectorAll('main, [role=main]')]
				.map((el) => el.innerText)
				.join(' ')
		),
		messageCount: messages.length,
		hash: location.hash,
		convInSidebar: document.body.innerText.includes('Write a short poem')
	};
});
console.log(JSON.stringify(state, null, 2));
console.log('ERRORS:', logs.length ? logs.slice(0, 6) : 'none');
await b.close();
