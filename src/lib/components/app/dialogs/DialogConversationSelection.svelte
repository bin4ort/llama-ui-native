<script>let { conversations, messageCountMap = new Map(), mode, onCancel, onConfirm, open = $bindable(false) } = $props();
let conversationSelectionRef = $state();
let previousOpen = $state(false);
$effect(() => {
    if (open && !previousOpen && conversationSelectionRef) {
        conversationSelectionRef.reset();
    }
    else if (!open && previousOpen) {
        onCancel();
    }
    previousOpen = open;
});
export {};
</script>

<Dialog.Root bind:open>
	<Dialog.Portal>
		<Dialog.Overlay class="z-1000000" />

		<Dialog.Content class="z-1000001 max-w-2xl">
			<Dialog.Header>
				<Dialog.Title>
					{t("Select Conversations to")} {mode === 'export' ? t('Export') : t('Import')}
				</Dialog.Title>

				<Dialog.Description>
					{#if mode === 'export'}
						{t("Choose which conversations you want to export. Selected conversations will be downloadedJSON file.")}
					{:else}
						{t("Choose which conversations you want to import. Selected conversations will be merged with your existing conversations.")}
					{/if}
				</Dialog.Description>
			</Dialog.Header>

			<ConversationSelection
				bind:this={conversationSelectionRef}
				isOpen={open}
				{conversations}
				{messageCountMap}
				{mode}
				{onCancel}
				{onConfirm}
			/>
		</Dialog.Content>
	</Dialog.Portal>
</Dialog.Root>
