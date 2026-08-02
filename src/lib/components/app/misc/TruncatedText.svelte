<script>let { text, class: className = '', showTooltip = true } = $props();
let textElement = $state();
let isTruncated = $state(false);
function checkTruncation() {
    if (textElement) {
        isTruncated = textElement.scrollWidth > textElement.clientWidth;
    }
}
$effect(() => {
    if (textElement) {
        checkTruncation();
        const observer = new ResizeObserver(checkTruncation);
        observer.observe(textElement);
        return () => observer.disconnect();
    }
});
export {};
</script>

{#if isTruncated && showTooltip}
	<Tooltip.Root>
		<Tooltip.Trigger class="{className} min-w-0">
			<span bind:this={textElement} class="block truncate">
				{text}
			</span>
		</Tooltip.Trigger>

		<Tooltip.Content class="z-[9999]">
			<p>{text}</p>
		</Tooltip.Content>
	</Tooltip.Root>
{:else}
	<span bind:this={textElement} class="{className} block min-w-0 truncate">
		{text}
	</span>
{/if}
