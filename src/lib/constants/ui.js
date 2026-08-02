import { Search, Settings, SquarePen } from '@lucide/svelte';
import McpLogo from '$lib/components/app/mcp/McpLogo.svelte';
import { ROUTES } from './routes';
export const FORK_TREE_DEPTH_PADDING = 8;
export const SYSTEM_MESSAGE_PLACEHOLDER = 'System message';
export const ICON_STRIP_TRANSITION_DURATION = 150;
export const ICON_STRIP_TRANSITION_DELAY_MULTIPLIER = 50;
/** Max height for tool-result code blocks (json / source / diff / streaming code). */
export const MAX_HEIGHT_CODE_BLOCK = '22rem';
export const SIDEBAR_ACTIONS_ITEMS = [
    { icon: SquarePen, tooltip: 'New chat', route: ROUTES.NEW_CHAT, keys: ['shift', 'cmd', 'o'] },
    { icon: Search, tooltip: 'Search', keys: ['cmd', 'k'] },
    {
        icon: McpLogo,
        tooltip: 'MCP Servers',
        route: ROUTES.MCP_SERVERS,
        activeRouteId: '/mcp-servers'
    },
    {
        icon: Settings,
        tooltip: 'Settings',
        route: `${ROUTES.SETTINGS}/general`,
        activeUrlIncludes: '#/settings'
    }
];
