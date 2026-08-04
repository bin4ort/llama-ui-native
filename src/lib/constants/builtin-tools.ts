/**
 * Built-in day-to-day tools (frontend-executed). Each tool has a settings
 * checkbox; enabled tools are added to the agentic loop's tool list.
 * See docs/day-to-day-tools-draft.md (v1 scope).
 */
import type { OpenAIToolDefinition } from '$lib/types';

export const BUILTIN_TOOL_NAMES = {
	CALCULATE: 'calculate',
	FETCH_URL: 'fetch_url',
	TO_TABLE: 'to_table',
	JSON_TOOL: 'json_tool',
	CLIPBOARD: 'clipboard',
	NOTIFY: 'notify',
	TODO_LIST: 'todo_list',
	WEATHER: 'weather',
	WIKIPEDIA: 'wikipedia',
	PLOT_CHART: 'plot_chart'
} as const;

/** Settings config keys (booleans) for each built-in tool. */
export const BUILTIN_TOOL_CONFIG_KEYS = {
	CALCULATE: 'toolCalculateEnabled',
	FETCH_URL: 'toolFetchUrlEnabled',
	TO_TABLE: 'toolToTableEnabled',
	JSON_TOOL: 'toolJsonEnabled',
	CLIPBOARD: 'toolClipboardEnabled',
	NOTIFY: 'toolNotifyEnabled',
	TODO_LIST: 'toolTodoEnabled',
	WEATHER: 'toolWeatherEnabled',
	WIKIPEDIA: 'toolWikipediaEnabled',
	PLOT_CHART: 'toolPlotChartEnabled'
} as const;

export type BuiltinToolName = (typeof BUILTIN_TOOL_NAMES)[keyof typeof BUILTIN_TOOL_NAMES];

export interface BuiltinToolMeta {
	name: string;
	/** i18n label key for the settings checkbox. */
	labelKey: string;
	/** Config key. */
	configKey: string;
	defaultEnabled: boolean;
}

export const BUILTIN_TOOLS: BuiltinToolMeta[] = [
	{ name: BUILTIN_TOOL_NAMES.CALCULATE, labelKey: 'Calculator', configKey: BUILTIN_TOOL_CONFIG_KEYS.CALCULATE, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.FETCH_URL, labelKey: 'Fetch URL', configKey: BUILTIN_TOOL_CONFIG_KEYS.FETCH_URL, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.TO_TABLE, labelKey: 'Table from data', configKey: BUILTIN_TOOL_CONFIG_KEYS.TO_TABLE, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.JSON_TOOL, labelKey: 'JSON helper', configKey: BUILTIN_TOOL_CONFIG_KEYS.JSON_TOOL, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.CLIPBOARD, labelKey: 'Clipboard', configKey: BUILTIN_TOOL_CONFIG_KEYS.CLIPBOARD, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.NOTIFY, labelKey: 'Notifications', configKey: BUILTIN_TOOL_CONFIG_KEYS.NOTIFY, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.TODO_LIST, labelKey: 'Task list', configKey: BUILTIN_TOOL_CONFIG_KEYS.TODO_LIST, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.WEATHER, labelKey: 'Weather', configKey: BUILTIN_TOOL_CONFIG_KEYS.WEATHER, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.WIKIPEDIA, labelKey: 'Wikipedia lookup', configKey: BUILTIN_TOOL_CONFIG_KEYS.WIKIPEDIA, defaultEnabled: true },
	{ name: BUILTIN_TOOL_NAMES.PLOT_CHART, labelKey: 'Chart', configKey: BUILTIN_TOOL_CONFIG_KEYS.PLOT_CHART, defaultEnabled: true }
];

/** Tool definition builders (model-facing; descriptions are intentionally English). */
export function buildCalculateToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.CALCULATE,
			description:
				'Evaluate a mathematical expression exactly. Supports + - * / ^ % parentheses, factorial (!), and functions sqrt, abs, sin, cos, tan, asin, acos, atan, ln, log10, exp, pow, round, floor, ceil, min, max. Constants: pi, e. Use this instead of doing arithmetic in prose.',
			parameters: {
				type: 'object',
				properties: { expression: { type: 'string', description: 'The mathematical expression to evaluate.' } },
				required: ['expression']
			}
		}
	};
}

export function buildFetchUrlToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.FETCH_URL,
			description:
				'Fetch a URL and return its visible text (HTML stripped). Useful to summarize articles, check docs pages or read web content. Only works with hosts that allow cross-origin requests; response is capped at 1 MB.',
			parameters: {
				type: 'object',
				properties: {
					url: { type: 'string', description: 'The URL to fetch (http/https).' },
					max_chars: { type: 'number', description: 'Optional limit on returned characters (default 12000).' }
				},
				required: ['url']
			}
		}
	};
}

export function buildToTableToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.TO_TABLE,
			description:
				'Convert CSV, TSV or JSON data into a clean markdown table. Use for budgets, logs, exports, rankings.',
			parameters: {
				type: 'object',
				properties: {
					data: { type: 'string', description: 'The raw data (CSV/TSV/JSON).' },
					format: { type: 'string', enum: ['auto', 'csv', 'tsv', 'json'], description: 'Input format (default auto).' },
					max_rows: { type: 'number', description: 'Cap on rows returned (default 100).' }
				},
				required: ['data']
			}
		}
	};
}

export function buildJsonToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.JSON_TOOL,
			description:
				'Validate, pretty-print or query JSON. Query paths use dot/bracket syntax, e.g. a.b[0].c. Use for configs, API responses and data exploration.',
			parameters: {
				type: 'object',
				properties: {
					action: { type: 'string', enum: ['validate', 'format', 'query'], description: 'What to do with the JSON.' },
					json: { type: 'string', description: 'The JSON text.' },
					path: { type: 'string', description: 'Query path, e.g. a.b[0].c (only for action=query).' }
				},
				required: ['action', 'json']
			}
		}
	};
}

export function buildClipboardToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.CLIPBOARD,
			description:
				'Read from or write to the system clipboard. Reading requires explicit user approval and may fail if the clipboard is unavailable.',
			parameters: {
				type: 'object',
				properties: {
					action: { type: 'string', enum: ['read', 'write'], description: 'read = get clipboard text, write = set clipboard text.' },
					text: { type: 'string', description: 'Text to write (only for action=write).' }
				},
				required: ['action']
			}
		}
	};
}

export function buildNotifyToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.NOTIFY,
			description:
				'Show a desktop notification. Use when a long-running task finishes or to alert the user while they may be looking elsewhere.',
			parameters: {
				type: 'object',
				properties: {
					title: { type: 'string', description: 'Notification title.' },
					body: { type: 'string', description: 'Notification body text.' }
				},
				required: ['title', 'body']
			}
		}
	};
}

export function buildTodoListToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.TODO_LIST,
			description:
				'Maintain a persistent to-do list for the current conversation. Actions: list, add, complete, clear.',
			parameters: {
				type: 'object',
				properties: {
					action: { type: 'string', enum: ['list', 'add', 'complete', 'clear'], description: 'Action to perform.' },
					text: { type: 'string', description: 'Task text (required for add).' },
					id: { type: 'string', description: 'Task id (required for complete).' }
				},
				required: ['action']
			}
		}
	};
}

export function buildWeatherToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.WEATHER,
			description:
				'Current weather conditions and a short forecast for a location (Open-Meteo, no API key). Pass latitude/longitude; a place name is resolved only if geocoding is available.',
			parameters: {
				type: 'object',
				properties: {
					latitude: { type: 'number', description: 'Latitude in decimal degrees.' },
					longitude: { type: 'number', description: 'Longitude in decimal degrees.' },
					location: { type: 'string', description: 'Optional location name for the report.' }
				},
				required: ['latitude', 'longitude']
			}
		}
	};
}

export function buildWikipediaToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.WIKIPEDIA,
			description:
				'Fetch the summary of a Wikipedia article for quick factual grounding. Returns an extract plus the canonical title.',
			parameters: {
				type: 'object',
				properties: {
					title: { type: 'string', description: 'Article title.' },
					lang: { type: 'string', description: 'Language code (default en).' }
				},
				required: ['title']
			}
		}
	};
}

export function buildPlotChartToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: BUILTIN_TOOL_NAMES.PLOT_CHART,
			description:
				'Render a line or bar chart from data as an inline SVG, plus a short text summary of the trend. Use for small data series.',
			parameters: {
				type: 'object',
				properties: {
					type: { type: 'string', enum: ['line', 'bar'], description: 'Chart type.' },
					labels: { type: 'array', items: { type: 'string' }, description: 'Category labels (x axis).' },
					values: { type: 'array', items: { type: 'number' }, description: 'Values (y axis).' },
					title: { type: 'string', description: 'Optional chart title.' }
				},
				required: ['type', 'labels', 'values']
			}
		}
	};
}

export function buildBuiltinToolDefinitions(config: Record<string, unknown>): OpenAIToolDefinition[] {
	const defs: OpenAIToolDefinition[] = [];
	const enabled = (key: string) => config[key] !== false;
	for (const meta of BUILTIN_TOOLS) {
		if (!enabled(meta.configKey)) continue;
		switch (meta.name) {
			case BUILTIN_TOOL_NAMES.CALCULATE:
				defs.push(buildCalculateToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.FETCH_URL:
				defs.push(buildFetchUrlToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.TO_TABLE:
				defs.push(buildToTableToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.JSON_TOOL:
				defs.push(buildJsonToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.CLIPBOARD:
				defs.push(buildClipboardToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.NOTIFY:
				defs.push(buildNotifyToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.TODO_LIST:
				defs.push(buildTodoListToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.WEATHER:
				defs.push(buildWeatherToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.WIKIPEDIA:
				defs.push(buildWikipediaToolDefinition());
				break;
			case BUILTIN_TOOL_NAMES.PLOT_CHART:
				defs.push(buildPlotChartToolDefinition());
				break;
		}
	}
	return defs;
}
