<script>let { showDeleteDialog, handleDeleteConfirm, showEmptyFileDialog, emptyFileNames, activeErrorDialog, handleErrorDialogOpenChange, fileUpload } = $props();
export {};
</script>

<DialogFileUploadError
	bind:open={fileUpload.showFileErrorDialog}
	fileErrorData={fileUpload.fileErrorData}
/>

<DialogConfirmation
	bind:open={showDeleteDialog}
	title={t("Delete Conversation")}
	description={t("Are you sure you want to delete this conversation? This action cannot be undone and will permanently remove all messages in this conversation.")}
	confirmText={t("Delete")}
	cancelText={t("Cancel")}
	variant="destructive"
	icon={Trash2}
	onConfirm={handleDeleteConfirm}
	onCancel={() => (showDeleteDialog = false)}
/>

<DialogEmptyFileAlert
	bind:open={showEmptyFileDialog}
	emptyFiles={emptyFileNames}
	onOpenChange={(open) => {
		if (!open) {
			emptyFileNames = [];
		}
	}}
/>

<DialogChatError
	message={activeErrorDialog?.message ?? ''}
	contextInfo={activeErrorDialog?.contextInfo}
	onOpenChange={handleErrorDialogOpenChange}
	open={Boolean(activeErrorDialog)}
	type={activeErrorDialog?.type ?? ErrorDialogType.SERVER}
/>
