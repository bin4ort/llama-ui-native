<script>let { isHealthChecking, onEdit, onRefresh, onDelete } = $props();
export {};
</script>

<div class="flex shrink-0 items-center gap-1">
	<Button variant="ghost" size="icon" class="h-7 w-7" onclick={onEdit} aria-label={t("Edit")}>
		<Pencil class="h-3.5 w-3.5" />
	</Button>

	<Button
		variant="ghost"
		size="icon"
		class="h-7 w-7"
		onclick={onRefresh}
		disabled={isHealthChecking}
		aria-label={t("Refresh")}
	>
		<RefreshCw class="h-3.5 w-3.5" />
	</Button>

	<Button
		variant="ghost"
		size="icon"
		class="hover:text-destructive-foreground h-7 w-7 text-destructive hover:bg-destructive/10"
		onclick={onDelete}
		aria-label={t("Delete")}
	>
		<Trash2 class="h-3.5 w-3.5" />
	</Button>
</div>
