<script>export let ariaLabel = 'Copy to clipboard';
export let canCopy = true;
export let text;
</script>

<ActionIcon
	icon={Copy}
	tooltip={ariaLabel}
	iconSize={ICON_CLASS_DEFAULT}
	disabled={!canCopy}
	onclick={() => canCopy && copyToClipboard(text)}
/>
