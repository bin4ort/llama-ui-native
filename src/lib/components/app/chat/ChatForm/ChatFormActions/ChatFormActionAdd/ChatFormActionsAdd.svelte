<script>let { disabled = false, hasAudioModality = false, hasVideoModality = false, hasMcpPromptsSupport = false, hasMcpResourcesSupport = false, hasVisionModality = false, onFileUpload, onMcpPromptClick, onMcpResourcesClick, onMcpSettingsClick, onSystemPromptClick } = $props();
export {};
</script>

{#if isMobile.current}
	<ChatFormActionAddSheet
		{disabled}
		{hasAudioModality}
		{hasVideoModality}
		{hasVisionModality}
		{hasMcpPromptsSupport}
		{hasMcpResourcesSupport}
		{onFileUpload}
		{onSystemPromptClick}
		{onMcpPromptClick}
		{onMcpResourcesClick}
	>
		{#snippet trigger({ disabled, onclick })}
			<ChatFormActionAddButton {disabled} {onclick} />
		{/snippet}
	</ChatFormActionAddSheet>
{:else}
	<ChatFormActionAddDropdown
		{disabled}
		{hasAudioModality}
		{hasVideoModality}
		{hasVisionModality}
		{hasMcpPromptsSupport}
		{hasMcpResourcesSupport}
		{onFileUpload}
		{onMcpPromptClick}
		{onMcpResourcesClick}
		{onMcpSettingsClick}
		{onSystemPromptClick}
	/>
{/if}
