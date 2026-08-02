<script>let { disabled = false, onclick } = $props();
export {};
</script>

<Tooltip.Root>
	<Tooltip.Trigger class="w-full">
		<Button
			class="file-upload-button md:h-8 md:w-8 h-9 w-9 rounded-full p-0"
			{disabled}
			{onclick}
			variant="secondary"
			type="button"
		>
			<span class="sr-only">{ATTACHMENT_TOOLTIP_TEXT}</span>

			<Plus class={ICON_CLASS_DEFAULT} />
		</Button>
	</Tooltip.Trigger>

	<Tooltip.Content>
		<p>{ATTACHMENT_TOOLTIP_TEXT}</p>
	</Tooltip.Content>
</Tooltip.Root>
