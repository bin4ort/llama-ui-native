import { ReasoningEffort } from '$lib/enums';
import { REASONING_EFFORT_LEVELS } from '$lib/constants/reasoning-effort';
import { REASONING_EFFORT_TOKENS } from '$lib/constants/reasoning-effort-tokens';
import { modelsStore, checkModelSupportsThinking, supportsThinking, propsCacheVersion, loadedModelIds } from '$lib/stores/models.svelte';
import { chatStore } from '$lib/stores/chat.svelte';
import { conversationsStore, activeMessages } from '$lib/stores/conversations.svelte';
import { isRouterMode } from '$lib/stores/server.svelte';
/**
 * Shared reactive state and helpers for the reasoning effort menu.
 *
 * Used by both the desktop dropdown (`ChatFormActionAddReasoningSubmenu`)
 * and the mobile sheet (`ChatFormActionAddSheet`) to avoid duplicating the
 * thinking-support derivation and the effort selection logic.
 */
export function useReasoningMenu() {
    const conversationModel = $derived(chatStore.getConversationModel(activeMessages()));
    // a router chat can carry reasoning from an earlier turn before the props
    // cache is primed, so a model that already produced thinking still qualifies
    const modelSupportsThinkingFromMessages = $derived.by(() => {
        const modelId = isRouterMode() ? modelsStore.selectedModelName || conversationModel : null;
        if (!modelId)
            return false;
        return conversationsStore.activeMessages.some((m) => m.role === 'assistant' && m.model === modelId && !!m.reasoningContent);
    });
    const modelSupportsThinking = $derived.by(() => {
        loadedModelIds();
        propsCacheVersion();
        if (isRouterMode()) {
            const modelId = modelsStore.selectedModelName || conversationModel;
            return checkModelSupportsThinking(modelId ?? '') || modelSupportsThinkingFromMessages;
        }
        return supportsThinking() || modelSupportsThinkingFromMessages;
    });
    const currentEffort = $derived(conversationsStore.getReasoningEffort());
    const thinkingEnabled = $derived(currentEffort !== ReasoningEffort.OFF && currentEffort !== ReasoningEffort.DEFAULT);
    return {
        get modelSupportsThinking() {
            return modelSupportsThinking;
        },
        get thinkingEnabled() {
            return thinkingEnabled;
        },
        get isOff() {
            return currentEffort === ReasoningEffort.OFF;
        },
        get currentEffort() {
            return currentEffort;
        },
        get levels() {
            return REASONING_EFFORT_LEVELS;
        },
        isSelected(level) {
            return currentEffort === level.value;
        },
        tokenLabel(level) {
            if (level.value === ReasoningEffort.DEFAULT)
                return 'Model default';
            const tokens = REASONING_EFFORT_TOKENS[level.value];
            if (tokens === undefined)
                return null;
            return tokens === -1 ? 'Unlimited' : `Max ${tokens.toLocaleString()} tokens`;
        },
        select(level) {
            conversationsStore.setReasoningEffort(level.value);
        }
    };
}
