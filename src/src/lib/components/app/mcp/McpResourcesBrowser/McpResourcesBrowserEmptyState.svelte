<script lang="ts">
	import { t, tr } from '$lib/stores/i18n.svelte';

	interface Props {
		isLoading: boolean;
	}

	let { isLoading }: Props = $props();
</script>

<div class="py-4 text-center text-sm text-muted-foreground">
	{#if isLoading}
		{tr.dict["Loading resources..."] || "Loading resources..."}
	{:else}
		{tr.dict["No resources available"] || "No resources available"}
	{/if}
</div>
