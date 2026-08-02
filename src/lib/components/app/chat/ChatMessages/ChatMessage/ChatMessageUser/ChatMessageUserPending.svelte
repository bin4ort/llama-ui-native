<script>import { useMessageEditContext } from '$lib/hooks/use-message-edit-context.svelte';
let { class: className = '', content, extras = [], onSendImmediately, onEdit, onDelete } = $props();
const editCtx = useMessageEditContext({
    getContent: () => content,
    getExtras: () => extras,
    onSave: (content, extras) => onEdit(content, extras)
});
</script>

<div
	aria-label={t("Pending user message")}
	class="group flex flex-col items-end gap-3 transition-opacity hover:opacity-80 md:gap-2 {className} sticky bottom-32"
	role="group"
>
	{#if editCtx.isEditing}
		<ChatMessageEditForm />
	{:else}
		<ChatMessageUserBubble
			{content}
			attachments={extras}
			textColorClass="text-muted-foreground"
			cardBgClass="dark:bg-primary/8"
			maxHeightStyle="overflow-wrap: anywhere; word-break: break-word;"
		/>

		<div class="max-w-[80%]">
			<div class="relative flex h-6 items-center justify-between">
				<div class="right-0 flex items-center gap-2 opacity-100 transition-opacity">
					<div
						class="pointer-events-auto inset-0 flex items-center gap-1 opacity-0 transition-all duration-150 group-hover:opacity-100"
					>
						<ActionIcon icon={Edit} tooltip={t("Edit")} onclick={editCtx.handleEdit} />
						<ActionIcon icon={Trash2} tooltip={t("Delete")} onclick={onDelete} />
						<ActionIcon icon={ArrowUp} tooltip={t("Send immediately")} onclick={onSendImmediately} />
					</div>
				</div>
			</div>
		</div>
	{/if}
</div>
