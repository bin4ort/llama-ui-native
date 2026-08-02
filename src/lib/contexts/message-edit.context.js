import { getContext, setContext } from 'svelte';
import { CONTEXT_KEY_MESSAGE_EDIT } from '$lib/constants';
const MESSAGE_EDIT_KEY = Symbol.for(CONTEXT_KEY_MESSAGE_EDIT);
/**
 * Sets the message edit context. Call this in the parent component (ChatMessage.svelte).
 */
export function setMessageEditContext(ctx) {
    return setContext(MESSAGE_EDIT_KEY, ctx);
}
/**
 * Gets the message edit context. Call this in child components.
 */
export function getMessageEditContext() {
    return getContext(MESSAGE_EDIT_KEY);
}
