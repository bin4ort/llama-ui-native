<script>import { t } from '$lib/stores/i18n.svelte';
import { buildConversationTree } from '$lib/stores/conversations.svelte';
let { class: className, filteredConversations, currentChatId, isSearchModeActive, searchQuery, isSelectionMode = false, selectedIds = new Set(), onSelect, onEdit, onDelete, onStop, onToggleSelect, onEnterSelectionMode, onSelectionClick, onRowMouseDown, visibleCount, allVisibleSelected, someVisibleSelected, allSelectedArePinned, pinStateIsMixed, onSelectAllToggle, onBulkPinToggle, onBulkExport, onBulkDelete, onCloseSelection } = $props();
let conversationTree = $derived(buildConversationTree(filteredConversations));
let pinnedConversations = $derived(conversationTree.filter(({ conversation }) => conversation.pinned));
let unpinnedConversations = $derived(conversationTree.filter(({ conversation }) => !conversation.pinned));
const recentEmptyMessage = $derived(searchQuery.length > 0 ? t("No results found") : t("No conversations yet"));
</script>

<div class="flex min-h-0 flex-1 flex-col">
	{#if isSearchModeActive}
		<SidebarNavigationSearchResults
			class={className}
			{searchQuery}
			{filteredConversations}
			{currentChatId}
			{onSelect}
			{onEdit}
			{onDelete}
			{onStop}
			{isSelectionMode}
			{selectedIds}
			{onToggleSelect}
			{onEnterSelectionMode}
			{onSelectionClick}
			{onRowMouseDown}
		/>
	{:else}
		{#if pinnedConversations.length > 0}
			<div class="py-2 flex whitespace-nowrap {className}">
				<div
					class="text-muted-foreground inline-flex h-8 shrink-0 items-center rounded-md px-2 text-xs font-medium gap-1"
				>
					<Pin class="h-3.5 w-3.5" />

					<span>{t("Pinned")}</span>
				</div>
			</div>

			<ul class="flex w-full min-w-0 flex-col gap-4 md:gap-1 {className}">
				{#each pinnedConversations as { conversation, depth } (conversation.id)}
					<li class="group/item relative mb-1 p-0">
						<SidebarNavigationConversationItem
							conversation={{
								id: conversation.id,
								name: conversation.name,
								lastModified: conversation.lastModified,
								currNode: conversation.currNode,
								forkedFromConversationId: conversation.forkedFromConversationId,
								pinned: conversation.pinned
							}}
							{depth}
							isActive={currentChatId === conversation.id}
							{isSelectionMode}
							isSelected={selectedIds.has(conversation.id)}
							{onSelect}
							{onEdit}
							{onDelete}
							{onStop}
							{onToggleSelect}
							{onEnterSelectionMode}
							{onSelectionClick}
							{onRowMouseDown}
						/>
					</li>
				{/each}
			</ul>
		{/if}

		<div class="mt-2 flex min-h-0 flex-1 flex-col gap-4 md:gap-0 whitespace-nowrap {className}">
			{#if filteredConversations.length > 0}
				<div
					class="text-muted-foreground flex h-8 shrink-0 items-center rounded-md px-2 text-xs font-medium"
				>
					{t("Recent conversations")}
				</div>
			{/if}

			<div class="min-h-0 flex-1 md:overflow-y-auto">
				<ul class="flex w-full min-w-0 flex-col gap-4 md:gap-0">
					{#each unpinnedConversations as { conversation, depth } (conversation.id)}
						<li class="group/item relative mb-1 p-0">
							<SidebarNavigationConversationItem
								conversation={{
									id: conversation.id,
									name: conversation.name,
									lastModified: conversation.lastModified,
									currNode: conversation.currNode,
									forkedFromConversationId: conversation.forkedFromConversationId,
									pinned: conversation.pinned
								}}
								{depth}
								isActive={currentChatId === conversation.id}
								{isSelectionMode}
								isSelected={selectedIds.has(conversation.id)}
								{onSelect}
								{onEdit}
								{onDelete}
								{onStop}
								{onToggleSelect}
								{onEnterSelectionMode}
								{onSelectionClick}
								{onRowMouseDown}
							/>
						</li>
					{/each}

					{#if unpinnedConversations.length === 0}
						<li class="px-2 py-4 text-center">
							<p class="mb-4 p-4 text-sm text-muted-foreground">
								{recentEmptyMessage}
							</p>
						</li>
					{/if}
				</ul>
			</div>
		</div>

		{#if isSelectionMode}
			<SidebarNavigationSelectionBar
				class="sticky top-0 z-10 m-2 mt-0"
				selectedCount={selectedIds.size}
				{visibleCount}
				{allVisibleSelected}
				{someVisibleSelected}
				someSelectedPinned={allSelectedArePinned}
				{pinStateIsMixed}
				{onSelectAllToggle}
				{onBulkPinToggle}
				{onBulkExport}
				{onBulkDelete}
				onClose={onCloseSelection}
			/>
		{/if}
	{/if}
</div>
