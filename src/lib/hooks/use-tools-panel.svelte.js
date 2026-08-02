import { CLI_FLAGS } from '$lib/constants';
import { SvelteSet } from 'svelte/reactivity';
import { ToolSource } from '$lib/enums';
import { conversationsStore } from '$lib/stores/conversations.svelte';
import { mcpStore } from '$lib/stores/mcp.svelte';
import { toolsStore } from '$lib/stores/tools.svelte';
/**
 * Shared reactive state and helpers for the tools panel UI.
 *
 * Used by both the desktop dropdown (`ChatFormActionAddToolsSubmenu`)
 * and the mobile sheet (`ChatFormActionAddSheet`) to avoid
 * duplicating group filtering, checked-state derivation, and favicon logic.
 */
export function useToolsPanel() {
    const expandedGroups = new SvelteSet();
    const groups = $derived(toolsStore.toolGroups);
    const activeGroups = $derived(groups.filter((g) => g.source !== ToolSource.MCP ||
        !g.serverId ||
        conversationsStore.isMcpServerEnabledForChat(g.serverId)));
    const totalToolCount = $derived(activeGroups.reduce((n, g) => n + g.tools.length, 0));
    const noToolsInfoMessage = $derived.by(() => {
        if (toolsStore.loading)
            return null;
        if (toolsStore.toolGroups.length > 0)
            return null;
        // Tools endpoint is unreachable (404) — server started without --tools
        if (toolsStore.isToolsEndpointUnreachable) {
            return `To enable Built-In Tools you need to run llama-server with ${CLI_FLAGS.TOOLS} all or ${CLI_FLAGS.TOOLS} <name> flag. To see MCP Tools you need to add / enable MCP Server(s).`;
        }
        // Other errors — return null so UI shows "Failed to load tools"
        if (toolsStore.error)
            return null;
        return `To enable Built-In Tools you need to run llama-server with ${CLI_FLAGS.TOOLS} all or ${CLI_FLAGS.TOOLS} <name> flag. To see MCP Tools you need to add / enable MCP Server(s).`;
    });
    function isGroupChecked(group) {
        return toolsStore.isGroupFullyEnabled(group);
    }
    function getEnabledToolCount(group) {
        return group.tools.filter((tool) => toolsStore.isToolEnabled(tool.key)).length;
    }
    function getFavicon(group) {
        if (group.source !== ToolSource.MCP || !group.serverId)
            return null;
        return mcpStore.getServerFavicon(group.serverId);
    }
    function isGroupDisabled(group) {
        return (group.source === ToolSource.MCP &&
            !!group.serverId &&
            !conversationsStore.isMcpServerEnabledForChat(group.serverId));
    }
    function toggleGroupExpanded(key) {
        if (expandedGroups.has(key)) {
            expandedGroups.delete(key);
        }
        else {
            expandedGroups.add(key);
        }
    }
    function toggleGroupByKey(key) {
        // Find current group by key to get up-to-date tool references
        const group = activeGroups.find((g) => g.key === key);
        if (!group)
            return;
        toolsStore.toggleGroup(group);
    }
    function handleOpen() {
        if (toolsStore.builtinTools.length === 0 && !toolsStore.loading) {
            toolsStore.fetchBuiltinTools();
        }
        mcpStore.runHealthChecksForServers(mcpStore.getServers().filter((s) => s.enabled));
    }
    return {
        expandedGroups,
        get groups() {
            return groups;
        },
        get activeGroups() {
            return activeGroups;
        },
        get totalToolCount() {
            return totalToolCount;
        },
        get noToolsInfoMessage() {
            return noToolsInfoMessage;
        },
        isGroupChecked,
        getEnabledToolCount,
        getFavicon,
        isGroupDisabled,
        toggleGroupExpanded,
        toggleGroupByKey,
        handleOpen
    };
}
