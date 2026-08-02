<script>let { isLoading } = $props();
export {};
</script>

<div class="py-4 text-center text-sm text-muted-foreground">
	{#if isLoading}
		{t("Loading resources...")}
	{:else}
		{t("No resources available")}
	{/if}
</div>
