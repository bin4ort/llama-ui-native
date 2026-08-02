import { getContext, setContext } from 'svelte';
import { CONTEXT_KEY_CHAT_SETTINGS_CONFIG } from '$lib/constants';
const CHAT_SETTINGS_CONFIG_KEY = Symbol.for(CONTEXT_KEY_CHAT_SETTINGS_CONFIG);
export function setChatSettingsConfigContext(ctx) {
    return setContext(CHAT_SETTINGS_CONFIG_KEY, ctx);
}
export function getChatSettingsConfigContext() {
    return getContext(CHAT_SETTINGS_CONFIG_KEY);
}
