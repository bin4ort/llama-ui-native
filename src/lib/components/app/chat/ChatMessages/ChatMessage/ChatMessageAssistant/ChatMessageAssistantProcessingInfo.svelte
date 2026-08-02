<script>let { modelLoadingText, processingState, position } = $props();
const marginClass = position === 'top' ? 'mt-6' : 'mt-4';
export {};
</script>

<div class="{marginClass} w-full max-w-3xl" in:fade>
	<div class="flex flex-col items-start gap-2">
		<span class="shimmer-text text-sm">
			{modelLoadingText ??
				processingState.getPromptProgressText() ??
				processingState.getProcessingMessage() ??
				t('Processing...')}
		</span>
	</div>
</div>
