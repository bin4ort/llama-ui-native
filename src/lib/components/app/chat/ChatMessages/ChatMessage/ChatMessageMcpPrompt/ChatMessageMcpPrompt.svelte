<script>import { getMessageEditContext } from '$lib/contexts';
let { class: className = '', message, mcpPrompt, siblingInfo = null, showDeleteDialog, deletionInfo, onCopy, onEdit, onDelete, onConfirmDelete, onNavigateToSibling, onShowDeleteDialogChange } = $props();
// Get edit context
const editCtx = getMessageEditContext();
</script>

<div
	aria-label={t("MCP Prompt message with actions")}
	class="group flex flex-col items-end gap-3 md:gap-2 {className}"
	role="group"
>
	{#if editCtx.isEditing}
		<ChatMessageEditForm />
	{:else}
		<ChatMessageMcpPromptContent
			prompt={mcpPrompt}
			variant={McpPromptVariant.MESSAGE}
			class="w-full max-w-[80%]"
		/>

		{#if message.timestamp}
			<div class="max-w-[80%]">
				<ChatMessageActionIcons
					actionsPosition="right"
					{deletionInfo}
					justify="end"
					{onConfirmDelete}
					{onCopy}
					{onDelete}
					{onEdit}
					{onNavigateToSibling}
					{onShowDeleteDialogChange}
					{siblingInfo}
					{showDeleteDialog}
					role={MessageRole.USER}
				/>
			</div>
		{/if}
	{/if}
</div>
