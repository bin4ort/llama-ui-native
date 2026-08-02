<script>import { mcpStore } from '$lib/stores/mcp.svelte';
import { mcpResources, mcpResourcesLoading } from '$lib/stores/mcp-resources.svelte';
import { SvelteMap, SvelteSet } from 'svelte/reactivity';
import { parseResourcePath } from '$lib/utils';
let { onSelect, onToggle, onTemplateSelect, selectedUris = new Set(), selectedTemplateUri, expandToUri, class: className } = $props();
let expandedServers = new SvelteSet();
let expandedFolders = new SvelteSet();
let searchQuery = $state('');
const resources = $derived(mcpResources());
const isLoading = $derived(mcpResourcesLoading());
const filteredResources = $derived.by(() => {
    if (!searchQuery.trim()) {
        return resources;
    }
    const query = searchQuery.toLowerCase();
    const filtered = new SvelteMap();
    for (const [serverName, serverRes] of resources.entries()) {
        const filteredResources = serverRes.resources.filter((r) => {
            return (r.title?.toLowerCase().includes(query) ||
                r.uri.toLowerCase().includes(query) ||
                serverName.toLowerCase().includes(query));
        });
        const filteredTemplates = serverRes.templates.filter((t) => {
            return (t.name?.toLowerCase().includes(query) ||
                t.title?.toLowerCase().includes(query) ||
                t.uriTemplate.toLowerCase().includes(query) ||
                serverName.toLowerCase().includes(query));
        });
        if (filteredResources.length > 0 || filteredTemplates.length > 0 || query.trim()) {
            filtered.set(serverName, {
                ...serverRes,
                resources: filteredResources,
                templates: filteredTemplates
            });
        }
    }
    return filtered;
});
$effect(() => {
    if (expandToUri && resources.size > 0) {
        autoExpandToResource(expandToUri);
    }
});
function autoExpandToResource(uri) {
    for (const [serverName, serverRes] of resources.entries()) {
        const resource = serverRes.resources.find((r) => r.uri === uri);
        if (resource) {
            expandedServers.add(serverName);
            const pathParts = parseResourcePath(uri);
            if (pathParts.length > 1) {
                let currentPath = '';
                for (let i = 0; i < pathParts.length - 1; i++) {
                    currentPath = `${currentPath}/${pathParts[i]}`;
                    const folderId = `${serverName}:${currentPath}`;
                    expandedFolders.add(folderId);
                }
            }
            break;
        }
    }
}
function toggleServer(serverName) {
    if (expandedServers.has(serverName)) {
        expandedServers.delete(serverName);
    }
    else {
        expandedServers.add(serverName);
    }
}
function toggleFolder(folderId) {
    if (expandedFolders.has(folderId)) {
        expandedFolders.delete(folderId);
    }
    else {
        expandedFolders.add(folderId);
    }
}
function handleRefresh() {
    mcpStore.fetchAllResources();
}
</script>

<div class={['flex flex-col gap-2', className]}>
	<McpResourcesBrowserHeader
		{isLoading}
		onRefresh={handleRefresh}
		onSearch={(q) => (searchQuery = q)}
		{searchQuery}
	/>

	<div class="flex flex-col gap-1">
		{#if filteredResources.size === 0}
			<McpResourcesBrowserEmptyState {isLoading} />
		{:else}
			{#each [...filteredResources.entries()] as [serverName, serverRes] (serverName)}
				<McpResourcesBrowserServerItem
					serverName={serverName}
					serverRes={serverRes}
					isExpanded={expandedServers.has(serverName)}
					{selectedUris}
					{selectedTemplateUri}
					{expandedFolders}
					onToggleServer={() => toggleServer(serverName)}
					onToggleFolder={toggleFolder}
					{onSelect}
					{onToggle}
					{onTemplateSelect}
					{searchQuery}
				/>
			{/each}
		{/if}
	</div>
</div>
