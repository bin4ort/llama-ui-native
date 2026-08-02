<script>import { untrack } from 'svelte';
import { activeConversation, activeMessages } from '$lib/stores/conversations.svelte';
import { chatStore, isChatStreaming, isLoading } from '$lib/stores/chat.svelte';
import { useContextGauge } from '$lib/hooks/use-context-gauge.svelte';
const gauge = useContextGauge();
$effect(() => {
    const conv = activeConversation();
    untrack(() => chatStore.setActiveProcessingConversation(conv?.id ?? null));
});
$effect(() => {
    const conv = activeConversation();
    const messages = activeMessages();
    if (!conv)
        return;
    if (isLoading() || isChatStreaming())
        return;
    if (messages.length === 0) {
        untrack(() => chatStore.clearProcessingState(conv.id));
        return;
    }
    untrack(() => chatStore.restoreProcessingStateFromMessages(messages, conv.id));
});
$effect(() => {
    gauge.startMonitoring();
});
</script>

<div
	role="button"
	tabindex="0"
	aria-label={t("Context usage")}
	data-context-gauge-trigger
	class="flex h-5 w-5 cursor-default items-center justify-center"
	onclick={gaugeTriggerClick}
	onkeydown={gaugeTriggerKeydown}
	onpointerdown={gaugeTriggerPointerDown}
	onpointerenter={gaugeTriggerEnter}
	onpointerleave={gaugeTriggerLeave}
>
	<ContextGaugeDial percent={gauge.contextPercent} level={gauge.colorLevel} />
</div>
