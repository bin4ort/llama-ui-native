/**
 * BuiltinToolsService — executors for the built-in day-to-day tools.
 * Called from the agentic loop (frontend tool source). Every tool returns a
 * plain-text result fed back to the model.
 */
import { BUILTIN_TOOL_NAMES } from '$lib/constants/builtin-tools';

export interface BuiltinToolContext {
	conversationId: string;
}

const MAX_FETCH_CHARS = 1_000_000;
const TODO_STORAGE_KEY = 'LlamaUi.todos';

function parseArgs(rawArgs: string): Record<string, unknown> {
	try {
		const parsed = JSON.parse(rawArgs || '{}');
		return parsed && typeof parsed === 'object' ? parsed : {};
	} catch {
		return {};
	}
}

function fail(message: string): { content: string; isError: true } {
	return { content: message, isError: true };
}

function ok(content: string): { content: string; isError: false } {
	return { content, isError: false };
}

function formatNumber(value: number): string {
	if (!Number.isFinite(value)) return String(value);
	// Trim float artifacts: 0.30000000000000004 -> 0.3
	const rounded = parseFloat(value.toPrecision(12));
	if (Number.isInteger(rounded) && Math.abs(rounded) < 1e21) return String(rounded);
	return String(rounded);
}

/* ---- calculate: safe recursive-descent expression parser ---- */

class CalcParser {
	private tokens: string[] = [];
	private pos = 0;

	constructor(private expr: string) {
		const re = /(\d+\.?\d*(?:[eE][+-]?\d+)?|[a-zA-Z]+|[-+*/^%()!,.])/g;
		this.tokens = this.expr.match(re) ?? [];
	}

	private peek(): string {
		return this.tokens[this.pos] ?? '';
	}

	private next(): string {
		return this.tokens[this.pos++] ?? '';
	}

	parse(): number {
		const value = this.parseAddSub();
		if (this.pos < this.tokens.length) throw new Error(`Unexpected token "${this.peek()}"`);
		return value;
	}

	private parseAddSub(): number {
		let value = this.parseMulDiv();
		while (true) {
			const op = this.peek();
			if (op === '+' || op === '-') {
				this.next();
				const rhs = this.parseMulDiv();
				value = op === '+' ? value + rhs : value - rhs;
			} else {
				return value;
			}
		}
	}

	private parseMulDiv(): number {
		let value = this.parsePow();
		while (true) {
			const op = this.peek();
			if (op === '*' || op === '/' || op === '%') {
				this.next();
				const rhs = this.parsePow();
				if (op === '*') value *= rhs;
				else if (op === '/') {
					if (rhs === 0) throw new Error('Division by zero');
					value /= rhs;
				} else {
					if (rhs === 0) throw new Error('Modulo by zero');
					value %= rhs;
				}
			} else {
				return value;
			}
		}
	}

	private parsePow(): number {
		const base = this.parseUnary();
		if (this.peek() === '^') {
			this.next();
			return Math.pow(base, this.parsePow());
		}
		return base;
	}

	private parseUnary(): number {
		const token = this.peek();
		if (token === '-') {
			this.next();
			return -this.parseUnary();
		}
		if (token === '+') {
			this.next();
			return this.parseUnary();
		}
		const value = this.parsePrimary();
		if (this.peek() === '!') {
			this.next();
			if (!Number.isInteger(value) || value < 0) throw new Error('Factorial needs a non-negative integer');
			let result = 1;
			for (let i = 2; i <= value; i++) result *= i;
			return result;
		}
		return value;
	}

	private parsePrimary(): number {
		const token = this.next();
		if (token === '(') {
			const value = this.parseAddSub();
			if (this.next() !== ')') throw new Error('Missing closing parenthesis');
			return value;
		}
		if (token === 'pi') return Math.PI;
		if (token === 'e') return Math.E;
		const number = Number(token);
		if (!Number.isNaN(number)) return number;
		// function call: name(arg, arg, ...)
		const args: number[] = [];
		if (this.peek() === '(') {
			this.next();
			if (this.peek() !== ')') {
				args.push(this.parseAddSub());
				while (this.peek() === ',') {
					this.next();
					args.push(this.parseAddSub());
				}
			}
			this.next(); // ')'
		}
		return this.applyFunction(token, args);
	}

	private applyFunction(name: string, args: number[]): number {
		const n = args[0];
		switch (name) {
			case 'sqrt': return Math.sqrt(n);
			case 'abs': return Math.abs(n);
			case 'sin': return Math.sin(n);
			case 'cos': return Math.cos(n);
			case 'tan': return Math.tan(n);
			case 'asin': return Math.asin(n);
			case 'acos': return Math.acos(n);
			case 'atan': return Math.atan(n);
			case 'ln': return Math.log(n);
			case 'log':
			case 'log10': return Math.log10(n);
			case 'exp': return Math.exp(n);
			case 'pow': return Math.pow(n, args[1]);
			case 'round': return Math.round(n);
			case 'floor': return Math.floor(n);
			case 'ceil': return Math.ceil(n);
			case 'min': return Math.min(...args);
			case 'max': return Math.max(...args);
			default: throw new Error(`Unknown function "${name}"`);
		}
	}
}

function calculate(expression: string): string {
	const result = new CalcParser(expression).parse();
	return formatNumber(result);
}

/* ---- fetch_url ---- */

async function stripHtml(html: string): Promise<string> {
	const doc = new DOMParser().parseFromString(html, 'text/html');
	doc.querySelectorAll('script, style, noscript, svg, nav, footer, header').forEach((el) => el.remove());
	return (doc.body?.textContent ?? '').replace(/\s+/g, ' ').trim();
}

async function fetchUrl(url: string, maxChars: number): Promise<string> {
	if (!/^https?:\/\//i.test(url)) throw new Error('Only http(s) URLs are supported');
	const response = await fetch(url, { signal: AbortSignal.timeout(15000) });
	if (!response.ok) throw new Error(`HTTP ${response.status} ${response.statusText}`);
	const contentType = response.headers.get('content-type') ?? '';
	const text = await response.text();
	if (text.length > MAX_FETCH_CHARS) throw new Error('Response exceeds the 1 MB cap');
	const content = contentType.includes('text/html')
		? await stripHtml(text)
		: text.replace(/\s+/g, ' ').trim();
	return content.slice(0, maxChars);
}

/* ---- to_table ---- */

function toTable(data: string, format: string, maxRows: number): string {
	let rows: string[][] = [];
	let header: string[] | null = null;

	if (format === 'json' || (format === 'auto' && data.trim().startsWith('['))) {
		const parsed = JSON.parse(data);
		if (!Array.isArray(parsed)) throw new Error('Expected a JSON array');
		const items = parsed.slice(0, maxRows);
		header = items.length > 0 ? Object.keys(items[0] ?? {}) : ['value'];
		rows = items.map((item) => header!.map((h) => String(item?.[h] ?? '')));
	} else {
		const delimiter = format === 'tsv' ? '\t' : format === 'csv' ? ',' : data.includes('\t') ? '\t' : ',';
		const lines = data.trim().split(/\r?\n/).filter((l) => l.trim()).slice(0, maxRows + 1);
		if (lines.length === 0) throw new Error('No data rows found');
		rows = lines.map((line) => line.split(delimiter).map((cell) => cell.trim()));
		header = rows[0];
		rows = rows.slice(1);
	}

	if (!header || header.length === 0) throw new Error('No columns found');
	const widths = header.map((_, col) => Math.max(...[header!, ...rows].map((r) => r[col]?.length ?? 0), 3));
	const fmt = (cells: string[]) => '| ' + cells.map((c, i) => (c ?? '').padEnd(widths[i])).join(' | ') + ' |';
	const sep = '| ' + widths.map((w) => '-'.repeat(w)).join(' | ') + ' |';
	return [fmt(header), sep, ...rows.map(fmt)].join('\n');
}

/* ---- json_tool ---- */

function jsonQuery(value: unknown, path: string): unknown {
	if (!path) return value;
	const parts = path.match(/[^.\[\]]+|\d+/g) ?? [];
	let current: unknown = value;
	for (const part of parts) {
		if (current === null || current === undefined) return undefined;
		if (Array.isArray(current)) current = current[Number(part)];
		else if (typeof current === 'object') current = (current as Record<string, unknown>)[part];
		else return undefined;
	}
	return current;
}

function jsonTool(action: string, json: string, path?: string): string {
	let parsed: unknown;
	try {
		parsed = JSON.parse(json);
	} catch (e) {
		throw new Error(`Invalid JSON: ${e instanceof Error ? e.message : String(e)}`);
	}
	if (action === 'validate') return 'Valid JSON';
	if (action === 'format') return JSON.stringify(parsed, null, 2);
	if (action === 'query') {
		const result = jsonQuery(parsed, path ?? '');
		return result === undefined ? 'No value at path' : JSON.stringify(result, null, 2);
	}
	throw new Error(`Unknown action "${action}"`);
}

/* ---- clipboard ---- */

async function clipboardAction(action: string, text?: string): Promise<string> {
	if (action === 'read') {
		try {
			const value = await navigator.clipboard.readText();
			return value ? value : '(clipboard is empty)';
		} catch {
			throw new Error('Clipboard read failed or permission denied');
		}
	}
	if (action === 'write') {
		if (!text) throw new Error('No text provided to write');
		try {
			await navigator.clipboard.writeText(text);
			return 'Copied to clipboard';
		} catch {
			throw new Error('Clipboard write failed');
		}
	}
	throw new Error(`Unknown action "${action}"`);
}

/* ---- notify ---- */

async function notify(title: string, body: string): Promise<string> {
	if (!('Notification' in window)) throw new Error('Notifications are not supported');
	if (Notification.permission === 'default') {
		const permission = await Notification.requestPermission();
		if (permission !== 'granted') throw new Error('Notification permission denied');
	}
	try {
		new Notification(title, { body });
		return `Notification shown: ${title}`;
	} catch {
		throw new Error('Failed to show notification');
	}
}

/* ---- todo_list ---- */

interface TodoItem {
	id: string;
	text: string;
	done: boolean;
}

function loadTodos(): Record<string, TodoItem[]> {
	try {
		return JSON.parse(localStorage.getItem(TODO_STORAGE_KEY) ?? '{}');
	} catch {
		return {};
	}
}

function saveTodos(todos: Record<string, TodoItem[]>): void {
	localStorage.setItem(TODO_STORAGE_KEY, JSON.stringify(todos));
}

function todoAction(conversationId: string, action: string, text?: string, id?: string): string {
	const all = loadTodos();
	const todos = all[conversationId] ?? [];
	switch (action) {
		case 'list':
			if (todos.length === 0) return 'No tasks yet.';
			return todos
				.map((t, i) => `${i + 1}. [${t.done ? 'x' : ' '}] ${t.text}${t.done ? ' (done)' : ''}`)
				.join('\n');
		case 'add': {
			if (!text?.trim()) throw new Error('Task text required');
			todos.push({ id: crypto.randomUUID(), text: text.trim(), done: false });
			all[conversationId] = todos;
			saveTodos(all);
			return `Task added: ${text.trim()}`;
		}
		case 'complete': {
			const todo = todos.find((t) => t.id === id);
			if (!todo) throw new Error(`Unknown task id "${id}"`);
			todo.done = true;
			all[conversationId] = todos;
			saveTodos(all);
			return `Completed: ${todo.text}`;
		}
		case 'clear':
			delete all[conversationId];
			saveTodos(all);
			return 'Task list cleared';
		default:
			throw new Error(`Unknown action "${action}"`);
	}
}

/* ---- weather (Open-Meteo, no key) ---- */

async function weather(latitude: number, longitude: number, location?: string): Promise<string> {
	const params = new URLSearchParams({
		latitude: String(latitude),
		longitude: String(longitude),
		current: 'temperature_2m,weather_code,wind_speed_10m',
		daily: 'weather_code,temperature_2m_max,temperature_2m_min',
		timezone: 'auto',
		forecast_days: '3'
	});
	const response = await fetch(`https://api.open-meteo.com/v1/forecast?${params}`, {
		signal: AbortSignal.timeout(15000)
	});
	if (!response.ok) throw new Error(`Weather API HTTP ${response.status}`);
	const data = await response.json();
	const current = data.current ?? {};
	const daily = data.daily ?? {};
	const label = location ? `${location} (${latitude.toFixed(2)}, ${longitude.toFixed(2)})` : `${latitude.toFixed(2)}, ${longitude.toFixed(2)}`;
	const lines = [
		`Weather for ${label}:`,
		`Now: ${current.temperature_2m ?? 'n/a'}°C, wind ${current.wind_speed_10m ?? 'n/a'} km/h`
	];
	for (let i = 0; i < (daily.time ?? []).length && i < 3; i++) {
		lines.push(
			`${daily.time[i]}: max ${daily.temperature_2m_max?.[i] ?? 'n/a'}°C / min ${daily.temperature_2m_min?.[i] ?? 'n/a'}°C`
		);
	}
	return lines.join('\n');
}

/* ---- wikipedia ---- */

async function wikipedia(title: string, lang: string): Promise<string> {
	const response = await fetch(
		`https://${lang}.wikipedia.org/api/rest_v1/page/summary/${encodeURIComponent(title)}`,
		{ signal: AbortSignal.timeout(15000) }
	);
	if (response.status === 404) throw new Error(`No Wikipedia article "${title}"`);
	if (!response.ok) throw new Error(`Wikipedia HTTP ${response.status}`);
	const data = await response.json();
	const extract = data.extract ?? '';
	return `Title: ${data.title ?? title}\n${extract.slice(0, 4000)}`;
}

/* ---- plot_chart (inline SVG) ---- */

function esc(value: string): string {
	return value.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function plotChart(type: string, labels: string[], values: number[], title?: string): string {
	if (!labels.length || labels.length !== values.length) throw new Error('labels and values must match in length');
	if (values.length > 100) throw new Error('Too many data points (max 100)');
	const W = 560;
	const H = 320;
	const padL = 48;
	const padR = 16;
	const padT = 36;
	const padB = 44;
	const max = Math.max(...values, 0);
	const min = Math.min(...values, 0);
	const range = max - min || 1;
	const plotW = W - padL - padR;
	const plotH = H - padT - padB;
	const x = (i: number) => padL + (labels.length === 1 ? plotW / 2 : (plotW * i) / (labels.length - 1));
	const y = (v: number) => padT + plotH - ((v - min) / range) * plotH;
	const grid = Array.from({ length: 5 }, (_, i) => {
		const v = min + (range * i) / 4;
		const gy = y(v);
		return `<line x1="${padL}" y1="${gy}" x2="${W - padR}" y2="${gy}" stroke="#e5e5e5" stroke-width="1"/><text x="${padL - 6}" y="${gy + 4}" text-anchor="end" font-size="11" fill="#888">${esc(formatNumber(v))}</text>`;
	}).join('');
	const body =
		type === 'bar'
			? values
					.map((v, i) => {
						const bw = Math.max(4, (plotW / labels.length) * 0.6);
						const bx = padL + (plotW * i) / labels.length + (plotW / labels.length - bw) / 2;
						const by = Math.min(y(v), y(0));
						const bh = Math.max(1, Math.abs(y(v) - y(0)));
						return `<rect x="${bx.toFixed(1)}" y="${by.toFixed(1)}" width="${bw.toFixed(1)}" height="${bh.toFixed(1)}" fill="#2563eb"/>`;
					})
					.join('')
			: values
					.map((v, i) => `${x(i).toFixed(1)},${y(v).toFixed(1)}`)
					.join(' ');
	const axisLine =
		min < 0 && max > 0 ? `<line x1="${padL}" y1="${y(0)}" x2="${W - padR}" y2="${y(0)}" stroke="#999" stroke-width="1"/>` : '';
	const labelsSvg = labels
		.map(
			(label, i) =>
				`<text x="${x(i).toFixed(1)}" y="${H - 12}" text-anchor="middle" font-size="11" fill="#888">${esc(label.slice(0, 14))}</text>`
		)
		.join('');
	const titleSvg = title
		? `<text x="${W / 2}" y="18" text-anchor="middle" font-size="14" font-weight="600" fill="#333">${esc(title)}</text>`
		: '';
	return [
		'```svg',
		`<svg xmlns="http://www.w3.org/2000/svg" width="${W}" height="${H}" viewBox="0 0 ${W} ${H}">${titleSvg}${grid}${axisLine}${
			type === 'line' ? `<polyline points="${body}" fill="none" stroke="#2563eb" stroke-width="2"/>` : body
		}${labelsSvg}</svg>`,
		'```',
		`Summary: ${values.length} points, min ${formatNumber(min)}, max ${formatNumber(max)}.`
	].join('\n');
}

/* ---- dispatcher ---- */

export interface BuiltinToolResult {
	content: string;
	isError: boolean;
}

export async function executeBuiltinTool(
	toolName: string,
	rawArgs: string,
	context: BuiltinToolContext
): Promise<BuiltinToolResult> {
	const args = parseArgs(rawArgs);
	try {
		switch (toolName) {
			case BUILTIN_TOOL_NAMES.CALCULATE:
				return ok(`= ${calculate(String(args.expression ?? ''))}`);
			case BUILTIN_TOOL_NAMES.FETCH_URL:
				return ok(await fetchUrl(String(args.url ?? ''), Number(args.max_chars ?? 12000)));
			case BUILTIN_TOOL_NAMES.TO_TABLE:
				return ok(toTable(String(args.data ?? ''), String(args.format ?? 'auto'), Number(args.max_rows ?? 100)));
			case BUILTIN_TOOL_NAMES.JSON_TOOL:
				return ok(jsonTool(String(args.action ?? ''), String(args.json ?? ''), args.path ? String(args.path) : undefined));
			case BUILTIN_TOOL_NAMES.CLIPBOARD:
				return ok(await clipboardAction(String(args.action ?? ''), args.text ? String(args.text) : undefined));
			case BUILTIN_TOOL_NAMES.NOTIFY:
				return ok(await notify(String(args.title ?? ''), String(args.body ?? '')));
			case BUILTIN_TOOL_NAMES.TODO_LIST:
				return ok(
					todoAction(
						context.conversationId,
						String(args.action ?? ''),
						args.text ? String(args.text) : undefined,
						args.id ? String(args.id) : undefined
					)
				);
			case BUILTIN_TOOL_NAMES.WEATHER:
				return ok(await weather(Number(args.latitude), Number(args.longitude), args.location ? String(args.location) : undefined));
			case BUILTIN_TOOL_NAMES.WIKIPEDIA:
				return ok(await wikipedia(String(args.title ?? ''), String(args.lang ?? 'en')));
			case BUILTIN_TOOL_NAMES.PLOT_CHART:
				return ok(
					plotChart(
						String(args.type ?? 'line'),
						Array.isArray(args.labels) ? args.labels.map(String) : [],
						Array.isArray(args.values) ? args.values.map(Number) : [],
						args.title ? String(args.title) : undefined
					)
				);
			default:
				return fail(`Unknown built-in tool "${toolName}"`);
		}
	} catch (error) {
		return fail(error instanceof Error ? error.message : String(error));
	}
}
