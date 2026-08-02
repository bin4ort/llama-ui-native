<script>let { instructions, class: className } = $props();
let isExpanded = $state(false);
export {};
</script>

{#if instructions}
	<Collapsible.Root bind:open={isExpanded} class={className}>
		<Collapsible.Trigger
			class="flex w-full items-center gap-1 text-xs text-muted-foreground hover:text-foreground"
		>
			{#if isExpanded}
				<ChevronDown class="h-3.5 w-3.5" />
			{:else}
				<ChevronRight class="h-3.5 w-3.5" />
			{/if}

			<span>{t("Server instructions")}</span>
		</Collapsible.Trigger>

		<Collapsible.Content class="mt-2">
			<p class="rounded bg-muted/50 p-2 text-xs text-muted-foreground">
				{instructions}
			</p>
		</Collapsible.Content>
	</Collapsible.Root>
{/if}
