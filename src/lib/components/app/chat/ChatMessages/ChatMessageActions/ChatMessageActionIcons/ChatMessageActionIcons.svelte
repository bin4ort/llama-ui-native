<script>import { activeConversation } from '$lib/stores/conversations.svelte';
let { actionsPosition, deletionInfo, justify, onCopy, onEdit, onConfirmDelete, onContinue, onDelete, onForkConversation, onNavigateToSibling, onShowDeleteDialogChange, onRegenerate, role, siblingInfo = null, showDeleteDialog, showRawOutputSwitch = false, rawOutputEnabled = false, onRawOutputToggle } = $props();
let showForkDialog = $state(false);
let forkName = $state('');
let forkIncludeAttachments = $state(true);
function handleConfirmDelete() {
    onConfirmDelete();
    onShowDeleteDialogChange(false);
}
function handleOpenForkDialog() {
    const conv = activeConversation();
    forkName = `Fork of ${conv?.name ?? 'Conversation'}`;
    forkIncludeAttachments = true;
    showForkDialog = true;
}
function handleConfirmFork() {
    onForkConversation?.({ name: forkName.trim(), includeAttachments: forkIncludeAttachments });
    showForkDialog = false;
}
</script>

<div class="relative {justify === 'start' ? 'mt-2' : ''} flex h-6 items-center justify-between">
	<div
		class="{actionsPosition === 'left'
			? 'left-0'
			: 'right-0'} flex items-center gap-2 opacity-100 transition-opacity"
	>
		{#if siblingInfo && siblingInfo.totalSiblings > 1}
			<ChatMessageActionIconsBranchingControls {siblingInfo} {onNavigateToSibling} />
		{/if}

		<div
			class="pointer-events-auto inset-0 flex items-center gap-1 opacity-100 transition-all duration-150"
		>
			<ActionIcon icon={Copy} tooltip={t("Copy")} onclick={onCopy} />

			{#if onEdit}
				<ActionIcon icon={Edit} tooltip={t("Edit")} onclick={onEdit} />
			{/if}

			{#if role === MessageRole.ASSISTANT && onRegenerate}
				<ActionIcon icon={RefreshCw} tooltip={t("Regenerate")} onclick={() => onRegenerate()} />
			{/if}

			{#if role === MessageRole.ASSISTANT && onContinue}
				<ActionIcon icon={ArrowRight} tooltip={t("Continue")} onclick={onContinue} />
			{/if}

			{#if onForkConversation}
				<ActionIcon icon={GitBranch} tooltip={t("Fork conversation")} onclick={handleOpenForkDialog} />
			{/if}

			<ActionIcon icon={Trash2} tooltip={t("Delete")} onclick={onDelete} />
		</div>
	</div>

	{#if showRawOutputSwitch}
		<div class="flex items-center gap-2">
			<span class="text-xs text-muted-foreground">{t('Show raw output')}</span>
			<Switch
				checked={rawOutputEnabled}
				onCheckedChange={(checked) => onRawOutputToggle?.(checked)}
			/>
		</div>
	{/if}
</div>

<DialogConfirmation
	bind:open={showDeleteDialog}
	title={t("Delete Message")}
	description={deletionInfo && deletionInfo.totalCount > 1
		? `This will delete ${deletionInfo.totalCount} messages including: ${deletionInfo.userMessages} user message${deletionInfo.userMessages > 1 ? 's' : ''} and ${deletionInfo.assistantMessages} assistant response${deletionInfo.assistantMessages > 1 ? 's' : ''}. All messages in this branch and their responses will be permanently removed. This action cannot be undone.`
		: 'Are you sure you want to delete this message? This action cannot be undone.'}
	confirmText={deletionInfo && deletionInfo.totalCount > 1
		? `Delete ${deletionInfo.totalCount} Messages`
		: 'Delete'}
	cancelText={t("Cancel")}
	variant="destructive"
	icon={Trash2}
	onConfirm={handleConfirmDelete}
	onCancel={() => onShowDeleteDialogChange(false)}
/>

<DialogConfirmation
	bind:open={showForkDialog}
	title={t("Fork Conversation")}
	description={t("Create a new conversation branching from this message.")}
	confirmText={t("Fork")}
	cancelText={t("Cancel")}
	icon={GitBranch}
	onConfirm={handleConfirmFork}
	onCancel={() => (showForkDialog = false)}
>
	<div class="flex flex-col gap-4 py-2">
		<div class="flex flex-col gap-2">
			<Label for="fork-name">{t("Title")}</Label>

			<Input
				id="fork-name"
				class="text-foreground"
				placeholder={t("Enter fork name")}
				type="text"
				bind:value={forkName}
			/>
		</div>

		<div class="flex items-center gap-2">
			<Checkbox
				id="fork-attachments"
				checked={forkIncludeAttachments}
				onCheckedChange={(checked) => {
					forkIncludeAttachments = checked === true;
				}}
			/>

			<Label for="fork-attachments" class="cursor-pointer text-sm font-normal">
				{t("Include all attachments")}
			</Label>
		</div>
	</div>
</DialogConfirmation>
