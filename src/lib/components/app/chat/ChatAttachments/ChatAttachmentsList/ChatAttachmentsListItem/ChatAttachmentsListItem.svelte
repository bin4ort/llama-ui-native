<script>let { class: className = '', imageClass = '', imageHeight = 'h-24', imageWidth = 'w-auto', item, limitToSingleRow = false, onFileRemove, onMcpResourcePreview, onPreview, readonly = false } = $props();
const scrollClasses = $derived(limitToSingleRow ? 'first:ml-4 last:mr-4' : '');
function toMcpResourceAttachment(extra, id) {
    return {
        id,
        resource: {
            uri: extra.uri,
            name: extra.name,
            title: extra.name,
            serverName: extra.serverName
        }
    };
}
export {};
</script>

{#if isMcpPrompt(item)}
	{@const mcpPrompt =
		item.attachment?.type === AttachmentType.MCP_PROMPT
			? (item.attachment)
			: item.uploadedFile?.mcpPrompt
				? {
						type: AttachmentType.MCP_PROMPT,
						name: item.name,
						serverName: item.uploadedFile.mcpPrompt.serverName,
						promptName: item.uploadedFile.mcpPrompt.promptName,
						content: item.textContent ?? '',
						arguments: item.uploadedFile.mcpPrompt.arguments
					}
				: null}
	{#if mcpPrompt}
		<ChatAttachmentsListItemMcpPrompt
			class="max-w-[300px] min-w-[200px] flex-shrink-0 {className} {scrollClasses}"
			prompt={mcpPrompt}
			{readonly}
			isLoading={item.isLoading}
			loadError={item.loadError}
			onRemove={onFileRemove ? () => onFileRemove(item.id) : undefined}
		/>
	{/if}
{:else if isMcpResource(item)}
	{@const mcpResource = item.attachment}

	<ChatAttachmentsListItemMcpResource
		class="flex-shrink-0 {className} {scrollClasses}"
		attachment={toMcpResourceAttachment(mcpResource, item.id)}
		onclick={() => onMcpResourcePreview?.(mcpResource)}
	/>
{:else if item.isImage && item.preview}
	<ChatAttachmentsListItemThumbnailImage
		class="flex-shrink-0 cursor-pointer {className} {scrollClasses}"
		id={item.id}
		name={item.name}
		preview={item.preview}
		{readonly}
		onRemove={onFileRemove}
		height={imageHeight}
		width={imageWidth}
		{imageClass}
		onclick={() => onPreview?.(item)}
	/>
{:else if isPdfFile(item.attachment, item.uploadedFile)}
	<ChatAttachmentsListItemThumbnailFile
		class="flex-shrink-0 cursor-pointer {className} {scrollClasses}"
		id={item.id}
		name={item.name}
		size={item.size}
		{readonly}
		onRemove={onFileRemove}
		textContent={item.textContent}
		attachment={item.attachment}
		uploadedFile={item.uploadedFile}
		onclick={() => onPreview?.(item)}
	/>
{:else}
	<ChatAttachmentsListItemThumbnailFile
		class="flex-shrink-0 cursor-pointer {className} {scrollClasses}"
		id={item.id}
		name={item.name}
		size={item.size}
		{readonly}
		onRemove={onFileRemove}
		textContent={item.textContent}
		attachment={item.attachment}
		uploadedFile={item.uploadedFile}
		onclick={() => onPreview?.(item)}
	/>
{/if}
