<script>let { class: className = '', disabled = false, hasAudioModality = false, isLoading = false, isRecording = false, onMicClick } = $props();
export {};
</script>

<div class="flex items-center gap-1 {className}">
	<Tooltip.Root>
		<Tooltip.Trigger>
			<Button
				class="h-8 w-8 rounded-full p-0 {isRecording
					? 'animate-pulse bg-red-500 text-white hover:bg-red-600'
					: ''}"
				disabled={disabled || isLoading || !hasAudioModality}
				onclick={onMicClick}
				type="button"
			>
				<span class="sr-only">{isRecording ? t('Stop recording') : t('Start recording')}</span>

				{#if isRecording}
					<Square class="{ICON_CLASS_DEFAULT} animate-pulse fill-white" />
				{:else}
					<Mic class={ICON_CLASS_DEFAULT} />
				{/if}
			</Button>
		</Tooltip.Trigger>

		{#if !hasAudioModality}
			<Tooltip.Content>
				<p>{t("Current model does not support audio")}</p>
			</Tooltip.Content>
		{/if}
	</Tooltip.Root>
</div>
