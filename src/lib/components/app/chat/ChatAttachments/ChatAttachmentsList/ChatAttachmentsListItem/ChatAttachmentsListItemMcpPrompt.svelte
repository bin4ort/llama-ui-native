<script>let { class: className = '', isLoading = false, loadError, onRemove, prompt, readonly = false } = $props();
export {};
</script>

<div class="group relative {className}">
	<ChatMessageMcpPromptContent
		{isLoading}
		{loadError}
		{prompt}
		variant={McpPromptVariant.ATTACHMENT}
	/>

	{#if !readonly && onRemove}
		<div
			class="absolute top-10 right-2 flex items-center justify-center opacity-0 transition-opacity group-hover:opacity-100"
		>
			<ActionIcon icon={X} tooltip="Remove" stopPropagationOnClick onclick={() => onRemove?.()} />
		</div>
	{/if}
</div>
