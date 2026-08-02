<script>import { AgenticSectionType } from '$lib/enums';
let { section, isStreaming = false } = $props();
const isPending = $derived(section.type === AgenticSectionType.TOOL_CALL_PENDING);
const isStreamingCall = $derived(section.type === AgenticSectionType.TOOL_CALL_STREAMING);
const showSpinner = $derived(isPending || (isStreamingCall && isStreaming));
function parseGetDatetimeMeta(toolResultString) {
    if (!toolResultString)
        return {};
    try {
        const parsed = JSON.parse(toolResultString);
        if (parsed && typeof parsed === 'object' && !Array.isArray(parsed)) {
            const obj = parsed;
            if (typeof obj.error === 'string')
                return { errorMessage: obj.error };
            if (typeof obj.result === 'string')
                return { dateString: obj.result.trim() };
        }
    }
    catch {
        return { dateString: toolResultString.trim() };
    }
    return {};
}
const dateMeta = $derived(parseGetDatetimeMeta(section.toolResult));
</script>

<div class="text-muted-foreground flex items-center gap-2 py-1.5">
	<Clock class="text-muted-foreground/60 h-3.5 w-3.5 shrink-0" />
	{#if showSpinner}
		<span class="text-foreground/80 text-sm font-medium">{t('Current time')}</span>
		<Loader2 class="text-muted-foreground/70 h-3 w-3 animate-spin" />
	{:else if dateMeta.errorMessage}
		<span class="text-foreground/80 text-sm font-medium">Current time&nbsp;</span>
		<span class="text-red-600 text-xs italic dark:text-red-400">-&nbsp;{dateMeta.errorMessage}</span
		>
	{:else if dateMeta.dateString}
		<span class="text-foreground/80 text-sm font-medium">{t('Current time is')}&nbsp;</span>
		<span class="font-mono text-foreground/90 text-sm">{dateMeta.dateString}</span>
	{:else}
		<span class="text-foreground/80 text-sm font-medium">Current time</span>
	{/if}
</div>
