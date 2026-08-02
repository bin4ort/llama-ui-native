<script>import { t } from '$lib/stores/i18n.svelte';
// Generic chrome shell shared by every per-tool block under
// `ChatMessageToolCall/`. Owns:
//   - the collapsible wrapper (defaults to CollapsibleContentBlock;
//     `exec_shell_command` swaps in CollapsibleTerminalBlock via the
//     `wrapper` prop);
//   - the icon, spinner state, and MCP favicon fallback chain;
//   - the status subtitle pill.
// Components supply only their `meta`, a title snippet, and a body
// snippet - everything around them is this single source of truth.
import { Loader2, Wrench } from '@lucide/svelte';
import { CollapsibleContentBlock } from '$lib/components/app';
import { ICON_CLASS_DEFAULT, ICON_CLASS_SPIN } from '$lib/constants/css-classes';
import { AgenticSectionType } from '$lib/enums';
import { getBuiltinToolUi } from '$lib/constants/built-in-tools';
import { mcpStore } from '$lib/stores/mcp.svelte';
let { section, open, isStreaming, meta, extraLiveStreaming = false, spinIconWhenActive = false, wrapper: Wrapper = CollapsibleContentBlock, title, titleSnippet, onToggle, children } = $props();
const isPending = $derived(section.type === AgenticSectionType.TOOL_CALL_PENDING);
const isStreamingCall = $derived(section.type === AgenticSectionType.TOOL_CALL_STREAMING);
const showSpinner = $derived(isPending || (isStreamingCall && isStreaming) || extraLiveStreaming);
const isCodeStreaming = $derived(isStreaming && (isPending || isStreamingCall));
const toolUi = $derived(getBuiltinToolUi(section.toolName));
const toolIcon = $derived(spinIconWhenActive && showSpinner ? Loader2 : (toolUi?.icon ?? Wrench));
const toolIconClass = $derived(spinIconWhenActive && showSpinner ? ICON_CLASS_SPIN : ICON_CLASS_DEFAULT);
// Drop the MCP favicon while the spinner is on so the title row
// signals "in flight" without being overwritten by server branding.
const mcpServerFavicon = $derived(showSpinner ? null : mcpStore.getServerFaviconForTool(section.toolName));
const iconUrl = $derived(showSpinner || (toolUi?.icon ?? null) || !mcpServerFavicon ? null : mcpServerFavicon);
function subtitleFor(errorMessage) {
    if (extraLiveStreaming)
        return t('streaming...');
    if (showSpinner)
        return t('executing...');
    if (errorMessage)
        return t('failed');
    if (isStreamingCall && !isStreaming)
        return t('incomplete');
    return undefined;
}
const subtitle = $derived(subtitleFor(meta?.errorMessage));
</script>

<Wrapper
	{open}
	class="my-2"
	icon={toolIcon}
	iconClass={toolIconClass}
	{iconUrl}
	{title}
	{titleSnippet}
	{subtitle}
	{onToggle}
>
	{@render children(meta, {
		isStreaming,
		isPending,
		isStreamingCall,
		isCodeStreaming
	})}
</Wrapper>
