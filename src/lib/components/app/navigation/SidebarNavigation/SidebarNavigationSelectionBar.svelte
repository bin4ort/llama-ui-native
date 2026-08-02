<script>import { t } from '$lib/stores/i18n.svelte';
let { class: className = '', selectedCount, visibleCount, allVisibleSelected, someVisibleSelected, someSelectedPinned, pinStateIsMixed, onSelectAllToggle, onBulkPinToggle, onBulkExport, onBulkDelete, onClose } = $props();
let showDeleteDialog = $state(false);
function handleDeleteClick() {
    showDeleteDialog = true;
}
function handleDeleteConfirm() {
    showDeleteDialog = false;
    onBulkDelete();
}
function handleDeleteCancel() {
    showDeleteDialog = false;
}
const hasSelection = $derived(selectedCount > 0);
const isMasterChecked = $derived(allVisibleSelected);
const isMasterIndeterminate = $derived(!allVisibleSelected && someVisibleSelected);
const pinTooltip = $derived(hasSelection
    ? pinStateIsMixed
        ? t("Unavailable for mixed state selection")
        : someSelectedPinned
            ? selectedCount === 1
                ? t("Unpin")
                : t("Unpin all")
            : selectedCount === 1
                ? t("Pin")
                : t("Pin all")
    : t("Pin"));
const pinDisabled = $derived(!hasSelection || pinStateIsMixed);
</script>

<div
	role="toolbar"
	aria-label={t("Bulk actions for selected conversations")}
	class="flex items-center gap-1.5 rounded-xl border border-border/50 bg-background/50 px-2 py-1.5 shadow-sm backdrop-blur-xl {className}"
>
	<label class="flex min-w-0 cursor-pointer items-center gap-2">
		<Checkbox
			checked={isMasterChecked}
			indeterminate={isMasterIndeterminate}
			onCheckedChange={onSelectAllToggle}
			aria-label={isMasterChecked ? t("Deselect all") : t("Select all")}
		/>

		<span class="truncate text-xs font-medium text-muted-foreground">
			{selectedCount} / {visibleCount} {t("selected")}
		</span>
	</label>

	<div class="ml-auto flex items-center gap-0.75">
		<ActionIcon
			icon={someSelectedPinned ? PinOff : Pin}
			tooltip={pinTooltip}
			tooltipSide={TooltipSide.TOP}
			disabled={pinDisabled}
			ariaLabel={pinTooltip}
			size="sm"
			iconSize="h-3.5 w-3.5"
			class="h-7 w-7 rounded-md bg-transparent backdrop-blur-none hover:bg-accent! {pinDisabled
				? 'cursor-not-allowed'
				: ''} {!pinDisabled ? 'opacity-100' : 'opacity-40'}"
			onclick={onBulkPinToggle}
		/>

		<ActionIcon
			icon={Download}
			tooltip={hasSelection ? t("Export") : t("Export")}
			tooltipSide={TooltipSide.TOP}
			disabled={!hasSelection}
			ariaLabel={t("Export selected")}
			size="sm"
			iconSize="h-3.5 w-3.5"
			class="h-7 w-7 rounded-md bg-transparent backdrop-blur-none hover:bg-accent! {hasSelection
				? 'opacity-100'
				: 'opacity-40'}"
			onclick={onBulkExport}
		/>

		<ActionIcon
			icon={Trash2}
			tooltip={t("Delete selected")}
			tooltipSide={TooltipSide.TOP}
			disabled={!hasSelection}
			ariaLabel={t("Delete selected")}
			size="sm"
			iconSize="h-3.5 w-3.5 text-destructive"
			class="h-7 w-7 rounded-md bg-transparent backdrop-blur-none hover:bg-destructive/10! dark:hover:bg-destructive/20! disabled:hover:bg-transparent {hasSelection
				? 'opacity-100'
				: 'opacity-40'}"
			onclick={handleDeleteClick}
		/>

		<div class="mx-1 h-4 w-px bg-border" aria-hidden="true"></div>

		<ActionIcon
			icon={X}
			tooltip={t("Exit bulk selection mode")}
			tooltipSide={TooltipSide.TOP}
			ariaLabel={t("Exit bulk selection mode")}
			size="sm"
			iconSize="h-3.5 w-3.5"
			class="h-7 w-7 rounded-md bg-transparent backdrop-blur-none hover:bg-accent!"
			onclick={onClose}
		/>
	</div>
</div>

<DialogConfirmation
	bind:open={showDeleteDialog}
	title={selectedCount === 1 ? t("Delete") + ' ' + selectedCount + ' ' + t("conversation") : t("Delete") + ' ' + selectedCount + ' ' + t("conversations")}
	description={selectedCount === 1
		? t("This action cannot be undone.") + ' ' + t("The selected conversation and its messages will be permanently removed, including any forks.")
		: t("This action cannot be undone.") + ' ' + t("The selected conversations and their messages will be permanently removed, including any forks.")}
	confirmText={selectedCount === 1 ? t("Delete") : t("Delete") + ' ' + selectedCount}
	cancelText={t("Cancel")}
	variant="destructive"
	icon={Trash2}
	onConfirm={handleDeleteConfirm}
	onCancel={handleDeleteCancel}
/>
