<script>let { isPromptPickerOpen, promptSearchQuery, isInlineResourcePickerOpen, resourceSearchQuery, onPromptPickerClose, onInlineResourcePickerClose, onInlineResourceSelect, onPromptLoadStart, onPromptLoadComplete, onPromptLoadError, onInlineResourceBrowse } = $props();
let promptPickerRef = $state(undefined);
let resourcePickerRef = $state(undefined);
/**
 * Delegates keyboard events to the active picker child.
 * Returns true if the event was handled.
 */
export function handleKeydown(event) {
    if (isPromptPickerOpen && promptPickerRef?.handleKeydown(event)) {
        return true;
    }
    if (isInlineResourcePickerOpen && resourcePickerRef?.handleKeydown(event)) {
        return true;
    }
    return false;
}
</script>

<ChatFormPickerMcpPrompts
	bind:this={promptPickerRef}
	isOpen={isPromptPickerOpen}
	searchQuery={promptSearchQuery}
	onClose={onPromptPickerClose}
	{onPromptLoadStart}
	{onPromptLoadComplete}
	{onPromptLoadError}
/>

<ChatFormPickerMcpResources
	bind:this={resourcePickerRef}
	isOpen={isInlineResourcePickerOpen}
	searchQuery={resourceSearchQuery}
	onClose={onInlineResourcePickerClose}
	onResourceSelect={onInlineResourceSelect}
	onBrowse={onInlineResourceBrowse}
/>
