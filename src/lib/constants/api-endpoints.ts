import { getApiBase } from '$lib/utils/api-base';

function api(path: string): string {
	return `${getApiBase()}${path}`;
}

export const API_MODELS = {
	get LIST() { return api('/v1/models'); },
	get LOAD() { return api('/models/load'); },
	get UNLOAD() { return api('/models/unload'); },
	get SSE() { return api('/models/sse'); }
};

export const API_CHAT = {
	get COMPLETIONS() { return api('/v1/chat/completions'); },
	get CONTROL() { return api('/v1/chat/completions/control'); }
};

export const API_SLOTS = {
	get LIST() { return api('/slots'); }
};

export const API_TOOLS = {
	get LIST() { return api('/tools'); },
	get EXECUTE() { return api('/tools'); }
};

export const API_STREAM = {
	get BASE() { return api('/v1/stream'); },
	get LOOKUP() { return api('/v1/streams/lookup'); }
};

export const CORS_PROXY_ENDPOINT = '/cors-proxy';
