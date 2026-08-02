<script>let { isLoading, onRefresh, onSearch, searchQuery = '' } = $props();
export {};
</script>

<div class="flex flex-col gap-2">
	<div class="mb-2 flex items-center gap-4">
		<SearchInput
			placeholder={t("Search resources...")}
			value={searchQuery}
			onInput={(value) => onSearch?.(value)}
		/>

		<Button
			variant="ghost"
			size="sm"
			class="h-8 w-8 p-0"
			onclick={onRefresh}
			disabled={isLoading}
			title={t("Refresh resources")}
		>
			{#if isLoading}
				<Loader2 class="{ICON_CLASS_DEFAULT} animate-spin" />
			{:else}
				<RefreshCw class={ICON_CLASS_DEFAULT} />
			{/if}
		</Button>
	</div>

	<h3 class="text-sm font-medium">{t("Available resources")}</h3>
</div>
