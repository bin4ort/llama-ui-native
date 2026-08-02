<script>let { open = $bindable(), currentTitle, value = $bindable(''), onConfirm, onCancel } = $props();
let inputRef = $state(null);
const canSubmit = $derived(value.trim().length > 0 && value.trim() !== currentTitle.trim());
$effect(() => {
    if (open) {
        value = currentTitle;
        queueMicrotask(() => {
            inputRef?.focus();
            inputRef?.select();
        });
    }
});
function handleOpenChange(newOpen) {
    if (!newOpen) {
        onCancel();
    }
}
function handleSubmit(event) {
    event.preventDefault();
    if (!canSubmit)
        return;
    value = value.trim();
    onConfirm();
}
export {};
</script>

<AlertDialog.Root bind:open onOpenChange={handleOpenChange}>
	<AlertDialog.Content>
		<AlertDialog.Header>
			<AlertDialog.Title class="flex items-center gap-2">
				<Pencil class="h-5 w-5" />
				{t("Rename conversation")}
			</AlertDialog.Title>

			<AlertDialog.Description>{t("Choose a new title for this conversation.")}</AlertDialog.Description>
		</AlertDialog.Header>

		<form onsubmit={handleSubmit} class="space-y-2 pt-2 pb-4">
			<label for="conversation-rename-input" class="text-sm font-medium text-muted-foreground">
				{t("Conversation title")}
			</label>

			<Input
				id="conversation-rename-input"
				bind:ref={inputRef}
				bind:value
				placeholder={t("Conversation title")}
				maxlength={200}
				autocomplete="off"
				autocorrect="off"
				spellcheck={false}
			/>
		</form>

		<AlertDialog.Footer>
			<AlertDialog.Cancel>{t("Cancel")}</AlertDialog.Cancel>

			<Button type="button" onclick={handleSubmit} disabled={!canSubmit}>{t("Save")}</Button>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
