<script>import { getAllLoadingChats } from '$lib/stores/chat.svelte';
import { conversationsStore } from '$lib/stores/conversations.svelte';
import { onMount } from 'svelte';
let { conversation, onDelete, onEdit, onSelect, onStop, onToggleSelect, onEnterSelectionMode, onSelectionClick, onRowMouseDown, isActive = false, isSelectionMode = false, isSelected = false, depth = 0 } = $props();
let renderActionsDropdown = $state(false);
let dropdownOpen = $state(false);
let isLoading = $derived(getAllLoadingChats().includes(conversation.id));
function handleEdit(event) {
    event.stopPropagation();
    onEdit?.(conversation.id);
}
function handleDelete(event) {
    event.stopPropagation();
    onDelete?.(conversation.id);
}
function handleStop(event) {
    event.stopPropagation();
    onStop?.(conversation.id);
}
function handleTogglePin() {
    conversationsStore.toggleConversationPin(conversation.id);
}
function handleEnterSelectionMode(event) {
    event.stopPropagation();
    onEnterSelectionMode?.(conversation.id);
}
function handleGlobalEditEvent(event) {
    const customEvent = event;
    if (customEvent.detail.conversationId === conversation.id && isActive) {
        handleEdit(event);
    }
}
function handleMouseLeave() {
    if (!dropdownOpen) {
        renderActionsDropdown = false;
    }
}
function handleMouseOver() {
    if (isSelectionMode)
        return;
    renderActionsDropdown = true;
}
function handleSelect(event) {
    if (isSelectionMode) {
        onSelectionClick?.(conversation.id, { shiftKey: event.shiftKey });
    }
    else {
        onSelect?.(conversation.id);
    }
}
function handleCheckboxClick(event) {
    event.stopPropagation();
    if (isSelectionMode) {
        onSelectionClick?.(conversation.id, { shiftKey: event.shiftKey });
    }
    else {
        onToggleSelect?.(conversation.id);
    }
}
function handleRowMouseDown(event) {
    onRowMouseDown?.(conversation.id, event);
}
function handleCheckboxKeydown(event) {
    if (event.key !== ' ' && event.key !== 'Enter')
        return;
    event.stopPropagation();
    event.preventDefault();
    if (isSelectionMode) {
        onSelectionClick?.(conversation.id, { shiftKey: event.shiftKey });
    }
    else {
        onToggleSelect?.(conversation.id);
    }
}
$effect(() => {
    if (!dropdownOpen) {
        renderActionsDropdown = false;
    }
});
onMount(() => {
    document.addEventListener('edit-active-conversation', handleGlobalEditEvent);
    return () => {
        document.removeEventListener('edit-active-conversation', handleGlobalEditEvent);
    };
});
</script>

<!-- svelte-ignore a11y_mouse_events_have_key_events -->
<button
	class="group flex min-h-9 w-full cursor-pointer items-center justify-between space-x-3 rounded-lg py-1.5 text-left transition-colors hover:bg-foreground/10 {isActive
		? 'bg-foreground/5 text-accent-foreground'
		: ''} {isSelected ? 'bg-primary/10 hover:bg-primary/15' : ''} {isSelectionMode
		? 'is-selection-mode'
		: ''} px-2"
	data-conversation-row={conversation.id}
	onclick={(e) => handleSelect(e)}
	onmouseover={handleMouseOver}
	onmouseleave={handleMouseLeave}
	onmousedown={(e) => handleRowMouseDown(e)}
	onfocusin={handleMouseOver}
	onfocusout={(e) => {
		if (!e.currentTarget.contains(e.relatedTarget)) {
			handleMouseLeave();
		}
	}}
>
	<div
		class="flex min-w-0 flex-1 items-center gap-2"
		style:padding-left="{depth * FORK_TREE_DEPTH_PADDING}px"
	>
		{#if isSelectionMode}
			<div
				class="shrink-0"
				onclick={(e) => handleCheckboxClick(e)}
				onkeydown={handleCheckboxKeydown}
				role="checkbox"
				aria-checked={isSelected}
				aria-label={isSelected ? t("Deselect") + ' ' + conversation.name : t("Select") + ' ' + conversation.name}
				tabindex="-1"
			>
				<Checkbox
					checked={isSelected}
					aria-label={isSelected ? t("Deselect") + ' ' + conversation.name : t("Select") + ' ' + conversation.name}
				/>
			</div>
		{/if}

		{#if depth > 0}
			<Tooltip.Root>
				<Tooltip.Trigger>
					<!-- prevent another nested button element -->
					{#snippet child({ props })}
						<a
							{...props}
							href={RouterService.chat(conversation.forkedFromConversationId)}
							class="flex shrink-0 items-center text-muted-foreground transition-colors hover:text-foreground"
						>
							<GitBranch class="h-3.5 w-3.5" />
						</a>
					{/snippet}
				</Tooltip.Trigger>

				<Tooltip.Content>
					<p>{t("See parent conversation")}</p>
				</Tooltip.Content>
			</Tooltip.Root>
		{/if}

		{#if isLoading}
			<Tooltip.Root>
				<Tooltip.Trigger>
					<div
						class="stop-button flex {ICON_CLASS_DEFAULT} shrink-0 cursor-pointer items-center justify-center rounded text-muted-foreground transition-colors hover:text-foreground"
						onclick={handleStop}
						onkeydown={(e) => e.key === 'Enter' && handleStop(e)}
						role="button"
						tabindex="0"
						aria-label={t("Stop generation")}
					>
						<Loader2 class="loading-icon h-3.5 w-3.5 animate-spin" />

						<Square class="stop-icon hidden h-3 w-3 fill-current text-destructive" />
					</div>
				</Tooltip.Trigger>

				<Tooltip.Content>
					<p>{t("Stop generation")}</p>
				</Tooltip.Content>
			</Tooltip.Root>
		{/if}

		<TruncatedText text={conversation.name} class="text-sm font-medium" showTooltip={false} />
	</div>

	{#if !isSelectionMode && renderActionsDropdown}
		<div class="actions flex items-center">
			<DropdownMenuActions
				triggerIcon={MoreHorizontal}
				triggerTooltip={t("More actions")}
				bind:open={dropdownOpen}
				actions={[
					{
						icon: conversation.pinned ? PinOff : Pin,
						label: conversation.pinned ? t("Unpin") : t("Pin"),
						onclick: (e) => {
							e.stopPropagation();
							handleTogglePin();
						}
					},
					{
						icon: Pencil,
						label: t("Edit"),
						onclick: handleEdit,
						shortcut: ['shift', 'cmd', 'e']
					},
					{
						icon: Download,
						label: t("Export"),
						onclick: (e) => {
							e.stopPropagation();
							conversationsStore.downloadConversation(conversation.id);
						},
						shortcut: ['shift', 'cmd', 's']
					},
					{
						icon: ListChecks,
						label: t("Select"),
						onclick: handleEnterSelectionMode
					},
					{
						icon: Trash2,
						label: t("Delete"),
						onclick: handleDelete,
						variant: 'destructive',
						shortcut: ['shift', 'cmd', 'd'],
						separator: true
					}
				]}
			/>
		</div>
	{/if}
</button>

<style>
	button {
		:global([data-slot='dropdown-menu-trigger']:not([data-state='open'])) {
			opacity: 0;
		}

		&:is(:hover) :global([data-slot='dropdown-menu-trigger']),
		&:focus-within :global([data-slot='dropdown-menu-trigger']) {
			opacity: 1;
		}
		@media (max-width: 768px) {
			:global([data-slot='dropdown-menu-trigger']) {
				opacity: 1 !important;
			}
		}

		&.is-selection-mode :global([data-slot='dropdown-menu-trigger']) {
			display: none !important;
		}

		.stop-button {
			:global(.stop-icon) {
				display: none;
			}

			:global(.loading-icon) {
				display: block;
			}
		}

		&:is(:hover) .stop-button {
			:global(.stop-icon) {
				display: block;
			}

			:global(.loading-icon) {
				display: none;
			}
		}
	}
</style>
