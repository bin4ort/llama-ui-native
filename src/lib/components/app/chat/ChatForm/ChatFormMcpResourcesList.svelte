<script>import { mcpStore } from '$lib/stores/mcp.svelte';
import { mcpResourceAttachments, mcpHasResourceAttachments } from '$lib/stores/mcp-resources.svelte';
let { class: className, onResourceClick } = $props();
const attachments = $derived(mcpResourceAttachments());
const hasAttachments = $derived(mcpHasResourceAttachments());
function handleRemove(attachmentId) {
    mcpStore.removeResourceAttachment(attachmentId);
}
function handleResourceClick(uri) {
    onResourceClick?.(uri);
}
</script>

{#if hasAttachments}
	<div class={className}>
		<HorizontalScrollCarousel gapSize="2">
			{#each attachments as attachment, i (attachment.id)}
				<ChatAttachmentsListItemMcpResource
					class={i === 0 ? 'ml-3' : ''}
					{attachment}
					onRemove={handleRemove}
					onclick={() => handleResourceClick(attachment.resource.uri)}
				/>
			{/each}
		</HorizontalScrollCarousel>
	</div>
{/if}
