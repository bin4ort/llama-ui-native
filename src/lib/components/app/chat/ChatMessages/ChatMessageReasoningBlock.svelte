<script>import { AgenticSectionType } from '$lib/enums';
import { REASONING_SCROLL_AT_BOTTOM_THRESHOLD_PX } from '$lib/constants/auto-scroll';
let { section, open, isStreaming, renderThinkingAsMarkdown, hasReasoningError = false, attachments, onToggle } = $props();
import { t } from '$lib/stores/i18n.svelte';
const REASONING_HEADER = t('Reasoning');
const REASONING_HEADER_PENDING = t('Reasoning...');
const REASONING_SUBTITLE_ERROR = t('Error');
const REASONING_SUBTITLE_CANCELLED = t('Cancelled');
const isPending = $derived(section.type === AgenticSectionType.REASONING_PENDING);
const title = $derived(isPending && isStreaming ? REASONING_HEADER_PENDING : REASONING_HEADER);
const subtitle = $derived.by(() => {
    if (isPending && !isStreaming) {
        return hasReasoningError ? REASONING_SUBTITLE_ERROR : REASONING_SUBTITLE_CANCELLED;
    }
    if (section.wasInterrupted) {
        return hasReasoningError ? REASONING_SUBTITLE_ERROR : REASONING_SUBTITLE_CANCELLED;
    }
    return isStreaming ? '' : undefined;
});
const shimmerTitle = $derived(isPending && isStreaming);
let scrollEl = $state();
const SCROLL_BOTTOM_THRESHOLD_PX = REASONING_SCROLL_AT_BOTTOM_THRESHOLD_PX;
let userScrolledUp = $state(false);
let lastScrollTop = 0;
let pendingFrame = null;
function isAtBottom() {
    if (!scrollEl)
        return false;
    return (scrollEl.scrollHeight - scrollEl.clientHeight - scrollEl.scrollTop <=
        SCROLL_BOTTOM_THRESHOLD_PX);
}
function scrollToBottomOnFrame() {
    if (pendingFrame !== null || !scrollEl || userScrolledUp)
        return;
    pendingFrame = requestAnimationFrame(() => {
        pendingFrame = null;
        // User may scroll between scheduling and paint.
        if (scrollEl && !userScrolledUp) {
            scrollEl.scrollTop = scrollEl.scrollHeight;
        }
    });
}
function handleScrollEvent() {
    if (!scrollEl)
        return;
    const isScrollingUp = scrollEl.scrollTop < lastScrollTop;
    if (isScrollingUp && !isAtBottom()) {
        userScrolledUp = true;
    }
    else if (isAtBottom()) {
        userScrolledUp = false;
    }
    lastScrollTop = scrollEl.scrollTop;
}
$effect(() => {
    void section.content;
    if (!scrollEl || !isPending || !isStreaming)
        return;
    scrollToBottomOnFrame();
});
$effect(() => {
    // Layout shifts that don't change section.content (markdown re-parse,
    // syntax-highlight settle, image loads).
    if (!scrollEl || !isPending || !isStreaming)
        return;
    const observer = new MutationObserver(() => scrollToBottomOnFrame());
    observer.observe(scrollEl, {
        childList: true,
        subtree: true,
        characterData: true
    });
    return () => observer.disconnect();
});
$effect(() => {
    // Pin to bottom at the start of each round.
    if (!isPending) {
        userScrolledUp = false;
        lastScrollTop = 0;
    }
});
</script>

<CollapsibleContentBlock
	{open}
	class="my-2"
	icon={Lightbulb}
	iconClass="h-3.5 w-3.5"
	{title}
	{subtitle}
	{shimmerTitle}
	{onToggle}
>
	<div
		bind:this={scrollEl}
		class="reasoning-content"
		class:is-streaming={isPending}
		onscroll={handleScrollEvent}
	>
		{#if renderThinkingAsMarkdown}
			<MarkdownContent content={section.content} class="text-muted-foreground" {attachments} />
		{:else}
			<div
				class="text-[13px] leading-relaxed wrap-break-word whitespace-pre-wrap text-muted-foreground"
			>
				{section.content}
			</div>
		{/if}
	</div>
</CollapsibleContentBlock>

<style>
	.reasoning-content.is-streaming {
		max-height: 28rem;
		overflow-y: auto;
		overscroll-behavior: contain;
		scrollbar-gutter: stable;
		padding-right: 0.25rem;
	}
</style>
