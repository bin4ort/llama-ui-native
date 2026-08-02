<script>import { copyToClipboard } from '$lib/utils';
let { class: className = '', icon: IconComponent, value, tooltipLabel } = $props();
function handleClick() {
    void copyToClipboard(String(value));
}
</script>

{#if tooltipLabel}
	<Tooltip.Root>
		<Tooltip.Trigger>
			<!-- prevent another nested button element -->
			{#snippet child({ props })}
				<BadgeInfo {...props} class={className} onclick={handleClick}>
					{#snippet icon()}
						<IconComponent class="h-3 w-3" />
					{/snippet}

					{value}
				</BadgeInfo>
			{/snippet}
		</Tooltip.Trigger>
		<Tooltip.Content>
			<p>{tooltipLabel}</p>
		</Tooltip.Content>
	</Tooltip.Root>
{:else}
	<BadgeInfo class={className} onclick={handleClick}>
		{#snippet icon()}
			<IconComponent class="h-3 w-3" />
		{/snippet}

		{value}
	</BadgeInfo>
{/if}
