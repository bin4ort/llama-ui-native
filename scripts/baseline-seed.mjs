/**
 * Shared Phase 0 baseline seed: a conversation + message tree matching the
 * exact IndexedDB schema of the current SvelteKit app ('LlamaUi' db).
 *
 * Used by scripts/screenshot-baseline.mjs and scripts/verify-baseline.mjs.
 * Reused by the vanilla rewrite (web/) for identical before/after data.
 */

export const MD = [
	'## Markdown sample',
	'',
	'Here is **bold**, *italic* and a [link](https://example.com).',
	'',
	'| Name | Value |',
	'|------|-------|',
	'| Alpha | 1 |',
	'| Beta | 2 |',
	'',
	'```python',
	'def hello():',
	'    print("hi")',
	'```',
	'',
	'Inline math $E=mc^2$ and display:',
	'',
	'$$\\int_0^1 x^2 dx = \\frac{1}{3}$$',
	'',
	'- item one',
	'- item two',
	''
].join('\n');

export const SEED_DATA = {
	conv: {
		id: 'baseline-conv-1',
		name: 'Baseline conversation',
		lastModified: Date.now(),
		currNode: 'm-assistant'
	},
	messages: [
		{ id: 'm-root', type: 'root', role: 'system', content: '', parent: null, children: ['m-persona'] },
		{
			id: 'm-persona',
			type: 'persona',
			role: 'system',
			content: 'You are a helpful assistant that answers in markdown.',
			parent: 'm-root',
			children: ['m-user']
		},
		{
			id: 'm-user',
			type: 'text',
			role: 'user',
			content: 'Show me a formatted markdown sample with a table, code and math.',
			parent: 'm-persona',
			children: ['m-assistant']
		},
		{ id: 'm-assistant', type: 'text', role: 'assistant', content: MD, parent: 'm-user', children: [] }
	]
};

/**
 * Runs in the page before app scripts: seeds the baseline conversation +
 * message tree into IndexedDB. Opens WITHOUT a version argument so it can
 * never block/upgrade the app's Dexie connection; stores are created only
 * when the database does not exist yet. Sets window.__seeded when done.
 */
export const SEED = (json) => {
	const { conv, messages } = JSON.parse(json);
	const req = indexedDB.open('LlamaUi');
	req.onupgradeneeded = () => {
		// Brand-new database (created by this call): create the stores the
		// app expects. Uses the same index set Dexie declares so no upgrade
		// is ever needed afterwards.
		const db = req.result;
		if (!db.objectStoreNames.contains('conversations')) {
			const c = db.createObjectStore('conversations', { keyPath: 'id' });
			for (const idx of ['lastModified', 'currNode', 'name']) c.createIndex(idx, idx);
		}
		if (!db.objectStoreNames.contains('messages')) {
			const m = db.createObjectStore('messages', { keyPath: 'id' });
			for (const idx of ['convId', 'type', 'role', 'timestamp', 'parent', 'children']) m.createIndex(idx, idx);
		}
	};
	req.onerror = () => {
		window.__seeded = true;
	};
	req.onsuccess = () => {
		const db = req.result;
		const ts = Date.now();
		const tx = db.transaction(['conversations', 'messages'], 'readwrite');
		tx.objectStore('conversations').put(conv);
		for (const m of messages) tx.objectStore('messages').put({ ...m, convId: conv.id, timestamp: ts });
		tx.oncomplete = () => {
			db.close();
			window.__seeded = true;
		};
	};
};
