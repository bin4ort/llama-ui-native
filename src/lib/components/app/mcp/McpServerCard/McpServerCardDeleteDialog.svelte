<script>let { open = $bindable(), displayName, onOpenChange, onConfirm } = $props();
export {};
</script>

<AlertDialog.Root bind:open {onOpenChange}>
	<AlertDialog.Content>
		<AlertDialog.Header>
			<AlertDialog.Title>{t("Delete Server")}</AlertDialog.Title>

			<AlertDialog.Description>
				{t("Are you sure you want to delete")} <strong>{displayName}</strong>? {t("This action cannot be undone.")}
			</AlertDialog.Description>
		</AlertDialog.Header>

		<AlertDialog.Footer>
			<AlertDialog.Cancel>{t("Cancel")}</AlertDialog.Cancel>

			<AlertDialog.Action
				class="text-destructive-foreground bg-destructive hover:bg-destructive/90"
				onclick={onConfirm}
			>
				{t("Delete")}
			</AlertDialog.Action>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
