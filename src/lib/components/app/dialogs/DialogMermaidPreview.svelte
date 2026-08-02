<script>let { open = $bindable(), svgHtml, onOpenChange } = $props();
export {};
</script>

<Dialog.Root bind:open {onOpenChange}>
	<Dialog.Content
		class="z-999999 grid max-h-full max-w-full! grid-rows-[1fr_auto] overflow-hidden p-0 md:h-[90vh] md:max-w-[90vw]!"
	>
		<MermaidPreview {svgHtml} />
	</Dialog.Content>
</Dialog.Root>
