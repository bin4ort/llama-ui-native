import { getContext, setContext } from 'svelte';
import { CONTEXT_KEY_CHAT_ACTIONS } from '$lib/constants';
const CHAT_ACTIONS_KEY = Symbol.for(CONTEXT_KEY_CHAT_ACTIONS);
export function setChatActionsContext(ctx) {
    return setContext(CHAT_ACTIONS_KEY, ctx);
}
export function getChatActionsContext() {
    return getContext(CHAT_ACTIONS_KEY);
}
