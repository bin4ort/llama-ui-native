<script>import { mcpStore } from '$lib/stores/mcp.svelte';
import { buildResourceTree } from './mcp-resources-browser';
let { serverName, serverRes, isExpanded, selectedUris, selectedTemplateUri, expandedFolders, onToggleServer, onToggleFolder, onSelect, onToggle, onTemplateSelect, searchQuery = '' } = $props();
let serverDisplayName = $derived(mcpStore.getServerDisplayName(serverName));
let serverFaviconUrl = $derived(mcpStore.getServerFavicon(serverName));
const hasResources = $derived(serverRes.resources.length > 0);
const hasTemplates = $derived(serverRes.templates.length > 0);
const hasContent = $derived(hasResources || hasTemplates);
const resourceTree = $derived(buildResourceTree(serverRes.resources, serverName, searchQuery));
const templateInfos = $derived(serverRes.templates.map((t) => ({
    uriTemplate: t.uriTemplate,
    name: t.name,
    title: t.title,
    description: t.description,
    mimeType: t.mimeType,
    serverName,
    annotations: t.annotations,
    icons: t.icons
})));
function handleResourceClick(resource, event) {
    onSelect?.(resource, event.shiftKey);
}
function handleCheckboxChange(resource, checked) {
    onToggle?.(resource, checked);
}
function isResourceSelected(resource) {
    return selectedUris.has(resource.uri);
}
</script>

{#snippet renderTreeNode(node, depth, parentPath)}
	{@const isFolder = !node.resource && node.children.size > 0}
	{@const folderId = `${serverName}:${parentPath}/${node.name}`}
	{@const isFolderExpanded = expandedFolders.has(folderId)}

	{#if isFolder}
		{@const folderCount = countTreeResources(node)}
		<Collapsible.Root open={isFolderExpanded} onOpenChange={() => onToggleFolder(folderId)}>
			<Collapsible.Trigger
				class="flex w-full items-center gap-2 rounded px-2 py-1 text-sm hover:bg-muted/50"
			>
				{#if isFolderExpanded}
					<ChevronDown class="h-3 w-3" />
				{:else}
					<ChevronRight class="h-3 w-3" />
				{/if}

				<FolderOpen class="h-3.5 w-3.5 text-muted-foreground" />

				<span class="font-medium">{node.name}</span>

				<span class="text-xs text-muted-foreground">({folderCount})</span>
			</Collapsible.Trigger>

			<Collapsible.Content>
				<div class="ml-4 flex flex-col gap-0.5 border-l border-border/50 pl-2">
					{#each sortTreeChildren( [...node.children.values()] ) as child (child.resource?.uri || `${serverName}:${parentPath}/${node.name}/${child.name}`)}
						{@render renderTreeNode(child, depth + 1, `${parentPath}/${node.name}`)}
					{/each}
				</div>
			</Collapsible.Content>
		</Collapsible.Root>
	{:else if node.resource}
		{@const resource = node.resource}
		{@const ResourceIcon = getResourceIcon(resource.mimeType, resource.uri)}
		{@const isSelected = isResourceSelected(resource)}
		{@const resourceDisplayName = resource.title || getDisplayName(node.name)}

		<div class="group flex w-full items-center gap-2">
			{#if onToggle}
				<Checkbox
					checked={isSelected}
					onCheckedChange={(checked: boolean | 'indeterminate') =>
						handleCheckboxChange(resource, checked === true)}
					class={ICON_CLASS_DEFAULT}
				/>
			{/if}

			<button
				class={[
					'flex flex-1 items-center gap-2 rounded px-2 py-1 text-left text-sm transition-colors',
					'hover:bg-muted/50',
					isSelected && 'bg-muted'
				]}
				onclick={(e) => handleResourceClick(resource, e)}
				title={resourceDisplayName}
			>
				<ResourceIcon class="h-3.5 w-3.5 shrink-0 text-muted-foreground" />

				<span class="min-w-0 flex-1 truncate text-left">
					{resourceDisplayName}
				</span>
			</button>
		</div>
	{/if}
{/snippet}

<Collapsible.Root open={isExpanded} onOpenChange={onToggleServer}>
	<Collapsible.Trigger
		class="flex w-full items-center gap-2 rounded px-2 py-1.5 text-sm hover:bg-muted/50"
	>
		{#if isExpanded}
			<ChevronDown class="h-3.5 w-3.5" />
		{:else}
			<ChevronRight class="h-3.5 w-3.5" />
		{/if}

		<span class="inline-flex flex-col items-start gap-1 text-left">
			<div class="inline-flex min-w-0 items-center gap-1.5">
				<McpServerIdentity
					displayName={serverDisplayName}
					faviconUrl={serverFaviconUrl}
					iconClass={ICON_CLASS_DEFAULT}
					showVersion={false}
				/>
			</div>

			<span class="text-xs text-muted-foreground">
				({serverRes.resources.length} {t("resource")}{serverRes.resources.length !== 1
					? t("s")
					: ''}{#if hasTemplates}, {serverRes.templates.length} {t("template")}{serverRes.templates
						.length !== 1
						? t("s")
						: ''}{/if})
			</span>
		</span>

		{#if serverRes.loading}
			<Loader2 class="ml-auto h-3 w-3 animate-spin text-muted-foreground" />
		{/if}
	</Collapsible.Trigger>

	<Collapsible.Content>
		<div class="ml-4 flex flex-col gap-0.5 border-l border-border/50 pl-2">
			{#if serverRes.error}
				<div class="py-1 text-xs text-red-500">
					{t("Error")}: {serverRes.error}
				</div>
			{:else if !hasContent}
				<div class="py-1 text-xs text-muted-foreground">{t("No resources")}</div>
			{:else}
				{#if hasResources}
					{#each sortTreeChildren( [...resourceTree.children.values()] ) as child (child.resource?.uri || `${serverName}:${child.name}`)}
						{@render renderTreeNode(child, 1, '')}
					{/each}
				{/if}

				{#if hasTemplates && onTemplateSelect}
					{#if hasResources}
						<div class="my-1 border-t border-border/30"></div>
					{/if}

					<div
						class="py-0.5 text-[11px] font-medium tracking-wide text-muted-foreground/70 uppercase"
					>
						{t("Templates")}
					</div>

					{#each templateInfos as template (template.uriTemplate)}
						<button
							class={[
								'flex w-full items-center gap-2 rounded px-2 py-1 text-left text-sm transition-colors',
								'hover:bg-muted/50',
								selectedTemplateUri === template.uriTemplate && 'bg-muted'
							]}
							onclick={() => onTemplateSelect(template)}
							title={template.uriTemplate}
						>
							<Braces class="h-3.5 w-3.5 shrink-0 text-muted-foreground" />

							<span class="min-w-0 flex-1 truncate text-left">
								{template.title || template.name}
							</span>
						</button>
					{/each}
				{/if}
			{/if}
		</div>
	</Collapsible.Content>
</Collapsible.Root>
