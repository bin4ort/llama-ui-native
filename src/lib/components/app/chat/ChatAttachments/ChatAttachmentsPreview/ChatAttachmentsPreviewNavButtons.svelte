<script>let { onPrev, onNext, show } = $props();
export {};
</script>

{#if show}
	<Button
		variant="secondary"
		size="icon"
		class="absolute top-1/2 left-4 z-10 h-8 w-8 -translate-y-1/2 rounded-full bg-background/5 p-0 text-white!"
		onclick={onPrev}
		aria-label={t("Previous")}
	>
		<ChevronLeft class="size-4" />
	</Button>

	<Button
		variant="secondary"
		size="icon"
		class="absolute top-1/2 right-4 z-10 h-8 w-8 -translate-y-1/2 rounded-full bg-background/5 p-0 text-white!"
		onclick={onNext}
		aria-label={t("Next")}
	>
		<ChevronRight class="size-4" />
	</Button>
{/if}
