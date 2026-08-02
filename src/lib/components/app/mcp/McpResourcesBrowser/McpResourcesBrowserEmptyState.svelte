<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';

	interface Props {
		isLoading: boolean;
	}

	let { isLoading }: Props = $props();
</script>

<div class="py-4 text-center text-sm text-muted-foreground">
	{#if isLoading}
		{t("Loading resources...")}
	{:else}
		{t("No resources available")}
	{/if}
</div>
