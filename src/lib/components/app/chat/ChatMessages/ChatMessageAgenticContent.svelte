<script>import { AgenticSectionType } from '$lib/enums';
import { deriveAgenticSections } from '$lib/utils';
import { agenticPendingPermissionRequest, agenticResolvePermission, agenticPendingContinueRequest, agenticResolveContinue, agenticLastError, agenticExecutingToolCallId } from '$lib/stores/agentic.svelte';
import { config } from '$lib/stores/settings.svelte';
let { message, toolMessages = [], isStreaming = false, isLastAssistantMessage = false } = $props();
let expandedStates = $state({});
const renderThinkingAsMarkdown = $derived(config().renderThinkingAsMarkdown);
const showThoughtInProgress = $derived(Boolean(config().showThoughtInProgress));
const alwaysShowToolCallContent = $derived(Boolean(config().alwaysShowToolCallContent));
const showMessageStats = $derived(Boolean(config().showMessageStats));
const showAgenticTurnStats = $derived(showMessageStats && Boolean(config().showAgenticTurnStats));
const hasReasoningError = $derived(isLastAssistantMessage ? !!agenticLastError(message.convId) : false);
let permissionDismissed = $state(false);
const pendingPermission = $derived(isStreaming && isLastAssistantMessage ? agenticPendingPermissionRequest(message.convId) : null);
let prevPendingRef = null;
$effect(() => {
    if (pendingPermission !== prevPendingRef) {
        prevPendingRef = pendingPermission;
        if (pendingPermission) {
            permissionDismissed = false;
        }
    }
});
function handlePermission(decision) {
    permissionDismissed = true;
    agenticResolvePermission(message.convId, decision);
}
let continueDismissed = $state(false);
const pendingContinue = $derived(isStreaming && isLastAssistantMessage ? agenticPendingContinueRequest(message.convId) : false);
let prevContinueRef = false;
$effect(() => {
    if (pendingContinue !== prevContinueRef) {
        prevContinueRef = pendingContinue;
        if (pendingContinue) {
            continueDismissed = false;
        }
    }
});
function handleContinue(shouldContinue) {
    continueDismissed = true;
    agenticResolveContinue(message.convId, shouldContinue);
}
const sections = $derived(deriveAgenticSections(message, toolMessages, [], isStreaming));
const currentlyExecutingToolCallId = $derived(isStreaming ? agenticExecutingToolCallId(message.convId) : null);
const turnGroups = $derived.by(() => {
    const groups = [];
    let currentTurn = [];
    let currentIndices = [];
    let prevWasTool = false;
    for (let i = 0; i < sections.length; i++) {
        const section = sections[i];
        const isTool = section.type === AgenticSectionType.TOOL_CALL ||
            section.type === AgenticSectionType.TOOL_CALL_PENDING ||
            section.type === AgenticSectionType.TOOL_CALL_STREAMING;
        if (!isTool && prevWasTool && currentTurn.length > 0) {
            groups.push({ sections: currentTurn, flatIndices: currentIndices });
            currentTurn = [];
            currentIndices = [];
        }
        currentTurn.push(section);
        currentIndices.push(i);
        prevWasTool = isTool;
    }
    if (currentTurn.length > 0) {
        groups.push({ sections: currentTurn, flatIndices: currentIndices });
    }
    return groups;
});
function getDefaultExpanded(section) {
    if (section.type === AgenticSectionType.TOOL_CALL ||
        section.type === AgenticSectionType.TOOL_CALL_PENDING ||
        section.type === AgenticSectionType.TOOL_CALL_STREAMING) {
        return alwaysShowToolCallContent;
    }
    if (section.type === AgenticSectionType.REASONING_PENDING) {
        return showThoughtInProgress;
    }
    return false;
}
function isExpanded(index, section) {
    if (expandedStates[index] !== undefined) {
        return expandedStates[index];
    }
    return getDefaultExpanded(section);
}
function toggleExpanded(index, section) {
    const currentState = isExpanded(index, section);
    expandedStates[index] = !currentState;
}
function buildTurnAgenticTimings(stats) {
    return {
        turns: 1,
        toolCallsCount: stats.toolCalls.length,
        toolsMs: stats.toolsMs,
        toolCalls: stats.toolCalls,
        llm: stats.llm
    };
}
</script>

{#snippet renderSection(section, index)}
	{#if section.type === AgenticSectionType.TEXT}
		<div class="agentic-text">
			<MarkdownContent content={section.content} attachments={message?.extra} />
		</div>
	{:else if section.type === AgenticSectionType.REASONING || section.type === AgenticSectionType.REASONING_PENDING}
		<ChatMessageReasoningBlock
			{section}
			open={isExpanded(index, section)}
			{isStreaming}
			{renderThinkingAsMarkdown}
			{hasReasoningError}
			attachments={message?.extra}
			onToggle={() => toggleExpanded(index, section)}
		/>
	{:else if section.type === AgenticSectionType.TOOL_CALL || section.type === AgenticSectionType.TOOL_CALL_PENDING || section.type === AgenticSectionType.TOOL_CALL_STREAMING}
		<ChatMessageToolCallBlock
			{section}
			open={isExpanded(index, section)}
			{isStreaming}
			isExecuting={section.toolCallId !== undefined &&
				section.toolCallId === currentlyExecutingToolCallId}
			attachments={message?.extra}
			onToggle={() => toggleExpanded(index, section)}
		/>
	{/if}
{/snippet}

<div class="agentic-content gap-2">
	{#if turnGroups.length > 1}
		{#each turnGroups as turn, turnIndex (turnIndex)}
			{@const turnStats = message?.timings?.agentic?.perTurn?.[turnIndex]}

			<div class="agentic-turn group/turn grid gap-2">
				{#each turn.sections as section, sIdx (turn.flatIndices[sIdx])}
					{@render renderSection(section, turn.flatIndices[sIdx])}
				{/each}

				{#if turnStats && showAgenticTurnStats}
					<div class="turn-stats transition-opacity duration-150 mt-1 mb-4">
						<ChatMessageStatistics
							promptTokens={turnStats.llm.prompt_n}
							promptMs={turnStats.llm.prompt_ms}
							predictedTokens={turnStats.llm.predicted_n}
							predictedMs={turnStats.llm.predicted_ms}
							agenticTimings={turnStats.toolCalls.length > 0
								? buildTurnAgenticTimings(turnStats)
								: undefined}
							initialView={ChatMessageStatsView.GENERATION}
							hideSummary
						/>
					</div>
				{/if}
			</div>
		{/each}
	{:else}
		{#each sections as section, index (index)}
			{@render renderSection(section, index)}
		{/each}
	{/if}

	{#if pendingPermission && !permissionDismissed}
		<ChatMessageActionCardPermissionRequest
			toolName={pendingPermission.toolName}
			serverLabel={pendingPermission.serverLabel}
			onDecision={handlePermission}
		/>
	{/if}

	{#if pendingContinue && !continueDismissed}
		<ChatMessageActionCardContinueRequest onDecision={handleContinue} />
	{/if}
</div>

<style>
	.agentic-content {
		display: flex;
		flex-direction: column;
		width: 100%;
		max-width: 48rem;
	}

	.agentic-content > :global(*),
	.agentic-turn > :global(*) {
		min-width: 0;
	}

	.agentic-text {
		width: 100%;
	}

	.turn-stats {
		border-top: 1px solid hsl(var(--muted) / 0.5);
	}
</style>
