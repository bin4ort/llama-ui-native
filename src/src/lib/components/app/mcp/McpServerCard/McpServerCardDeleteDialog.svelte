<script lang="ts">
	import { t, tr } from '$lib/stores/i18n.svelte';
	import * as AlertDialog from '$lib/components/ui/alert-dialog';

	interface Props {
		open: boolean;
		displayName: string;
		onOpenChange: (open: boolean) => void;
		onConfirm: () => void;
	}

	let { open = $bindable(), displayName, onOpenChange, onConfirm }: Props = $props();
</script>

<AlertDialog.Root bind:open {onOpenChange}>
	<AlertDialog.Content>
		<AlertDialog.Header>
			<AlertDialog.Title>{tr.dict["Delete Server"] || "Delete Server"}</AlertDialog.Title>

			<AlertDialog.Description>
				{tr.dict["Are you sure you want to delete"] || "Are you sure you want to delete"} <strong>{displayName}</strong>? {tr.dict["This action cannot be undone."] || "This action cannot be undone."}
			</AlertDialog.Description>
		</AlertDialog.Header>

		<AlertDialog.Footer>
			<AlertDialog.Cancel>{tr.dict["Cancel"] || "Cancel"}</AlertDialog.Cancel>

			<AlertDialog.Action
				class="text-destructive-foreground bg-destructive hover:bg-destructive/90"
				onclick={onConfirm}
			>
				{tr.dict["Delete"] || "Delete"}
			</AlertDialog.Action>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
