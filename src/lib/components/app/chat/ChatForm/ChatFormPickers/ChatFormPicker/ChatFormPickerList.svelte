<script>import { t } from '$lib/stores/i18n.svelte';
let { items, isLoading, selectedIndex, searchQuery = $bindable(), showSearchInput, searchPlaceholder = t('Search...'), emptyMessage = t('No items available'), itemKey, item, skeleton, footer } = $props();
let listContainer = $state(null);
$effect(() => {
    if (listContainer && selectedIndex >= 0 && selectedIndex < items.length) {
        const selectedElement = listContainer.querySelector(`[data-picker-index="${selectedIndex}"]`);
        if (selectedElement) {
            selectedElement.scrollIntoView({
                behavior: 'smooth',
                block: 'center',
                inline: 'nearest'
            });
        }
    }
});
</script>

<ScrollArea>
	{#if showSearchInput}
		<div class="absolute top-0 right-0 left-0 z-10 p-2 pb-0">
			<SearchInput placeholder={searchPlaceholder} bind:value={searchQuery} />
		</div>
	{/if}

	<div
		bind:this={listContainer}
		class={[`${CHAT_FORM_POPOVER_MAX_HEIGHT} p-2`, showSearchInput && 'pt-13']}
	>
		{#if isLoading}
			{#if skeleton}
				{@render skeleton()}
			{/if}
		{:else if items.length === 0}
			<div class="py-6 text-center text-sm text-muted-foreground">{emptyMessage}</div>
		{:else}
			{#each items as itemData, index (itemKey(itemData, index))}
				{@render item(itemData, index, index === selectedIndex)}
			{/each}
		{/if}
	</div>

	{#if footer}
		{@render footer()}
	{/if}
</ScrollArea>
