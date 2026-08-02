<script>import { FileTypeText } from '$lib/enums';
let { code, language, disabled = false, onPreview } = $props();
const showPreview = $derived(language?.toLowerCase() === FileTypeText.HTML);
</script>

<div class="code-block-actions">
	<ActionIconCopyToClipboard
		text={code}
		canCopy={!disabled}
		ariaLabel={disabled ? 'Code incomplete' : 'Copy code'}
	/>

	{#if showPreview}
		<ActionIcon
			icon={Eye}
			tooltip={disabled ? 'Code incomplete' : 'Preview code'}
			{disabled}
			onclick={() => onPreview(code, language)}
		/>
	{/if}
</div>
