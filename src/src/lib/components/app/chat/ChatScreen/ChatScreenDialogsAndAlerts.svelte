<script lang="ts">
	import { t, tr } from '$lib/stores/i18n.svelte';

	import { Trash2 } from '@lucide/svelte';
	import { ErrorDialogType } from '$lib/enums';
	import {
		DialogChatError,
		DialogConfirmation,
		DialogEmptyFileAlert,
		DialogFileUploadError
	} from '$lib/components/app';

	let {
		showDeleteDialog,
		handleDeleteConfirm,
		showEmptyFileDialog,
		emptyFileNames,
		activeErrorDialog,
		handleErrorDialogOpenChange,
		fileUpload
	} = $props();
</script>

<DialogFileUploadError
	bind:open={fileUpload.showFileErrorDialog}
	fileErrorData={fileUpload.fileErrorData}
/>

<DialogConfirmation
	bind:open={showDeleteDialog}
	title={tr.dict["Delete Conversation"] || "Delete Conversation"}
	description={tr.dict["Are you sure you want to delete this conversation? This action cannot be undone and will permanently remove all messages in this conversation."] || "Are you sure you want to delete this conversation? This action cannot be undone and will permanently remove all messages in this conversation."}
	confirmText={tr.dict["Delete"] || "Delete"}
	cancelText={tr.dict["Cancel"] || "Cancel"}
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
