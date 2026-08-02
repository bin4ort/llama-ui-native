<script>import { t } from '$lib/stores/i18n.svelte';
let { placeholder = t("Search..."), searchValue = $bindable(''), onSearchChange, onSearchKeyDown, emptyMessage = t("No items found"), isEmpty = false, children, footer } = $props();
</script>

<div class="sticky top-0 z-10 mb-2 bg-popover p-1 pt-2">
	<SearchInput
		{placeholder}
		bind:value={searchValue}
		onInput={onSearchChange}
		onKeyDown={onSearchKeyDown}
	/>
</div>

<div class="overflow-y-auto">
	{@render children()}

	{#if isEmpty}
		<div class="px-2 py-3 text-center text-sm text-muted-foreground">{emptyMessage}</div>
	{/if}
</div>

{#if footer}
	<DropdownMenu.Separator />

	{@render footer()}
{/if}
