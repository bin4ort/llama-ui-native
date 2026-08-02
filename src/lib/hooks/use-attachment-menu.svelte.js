import { page } from '$app/state';
import { AttachmentAction } from '$lib/enums';
/**
 * useAttachmentMenu - Shared logic for attachment menu components.
 *
 * Encapsulates the modality-flag checks and callback wrapping that is
 * identical across the desktop dropdown (`ChatFormActionAddDropdown`)
 * and the mobile sheet (`ChatFormActionAddSheet`).
 *
 * @param getFlags   - Getter returning the current modality capability flags.
 * @param getCallbacks - Getter returning the raw action callbacks from props.
 * @param close      - Function that dismisses the hosting UI element (dropdown / sheet).
 */
export function useAttachmentMenu(getFlags, getCallbacks, close) {
    const modalityFlags = $derived(getFlags());
    const callbacks = $derived.by(() => {
        const cbs = getCallbacks();
        const wrap = (fn) => () => {
            close();
            fn?.();
        };
        return {
            [AttachmentAction.FILE_UPLOAD]: wrap(cbs.onFileUpload),
            [AttachmentAction.SYSTEM_PROMPT_CLICK]: wrap(cbs.onSystemPromptClick),
            [AttachmentAction.MCP_PROMPT_CLICK]: wrap(cbs.onMcpPromptClick),
            [AttachmentAction.MCP_RESOURCES_CLICK]: wrap(cbs.onMcpResourcesClick)
        };
    });
    function isItemEnabled(enabledWhen) {
        if (!enabledWhen || enabledWhen === 'always')
            return true;
        return !!modalityFlags[enabledWhen];
    }
    function isItemVisible(visibleWhen) {
        if (!visibleWhen)
            return true;
        return !!modalityFlags[visibleWhen];
    }
    function getSystemMessageTooltip() {
        return !page.params.id
            ? 'Add custom system message for a new conversation'
            : 'Inject custom system message at the beginning of the conversation';
    }
    return {
        get callbacks() {
            return callbacks;
        },
        isItemEnabled,
        isItemVisible,
        getSystemMessageTooltip
    };
}
