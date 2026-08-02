export enum ColorMode {
	LIGHT = 'light',
	SNOW = 'snow',
	DARK = 'dark',
	SYSTEM = 'system',
	NORD = 'nord',
	DRACULA = 'dracula',
	COBALT = 'cobalt',
	SOLARIZED = 'solarized',
	GRUVBOX = 'gruvbox',
	GRUVBOX_LIGHT = 'gruvbox-light',
	AMOLED = 'amoled',
	IONIZED_PURPLE = 'ionized-purple',
	IONIZED_RED = 'ionized-red',
	IONIZED_CYAN = 'ionized-cyan'
}

export enum TooltipSide {
	TOP = 'top',
	RIGHT = 'right',
	BOTTOM = 'bottom',
	LEFT = 'left'
}

/**
 * MCP prompt display variant
 */
export enum McpPromptVariant {
	MESSAGE = 'message',
	ATTACHMENT = 'attachment'
}

/**
 * URL prefixes for protocol detection
 */
export enum UrlProtocol {
	DATA = 'data:',
	HTTP = 'http:',
	HTTPS = 'https:',
	WEBSOCKET = 'ws:',
	WEBSOCKET_SECURE = 'wss:'
}

export enum HtmlInputType {
	FILE = 'file'
}
