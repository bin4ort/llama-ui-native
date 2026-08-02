<script>let { onclick } = $props();
export {};
</script>

<div class="pointer-events-auto flex justify-center relative h-0">
	<ActionIcon
		icon={ArrowDown}
		{onclick}
		ariaLabel={t("Scroll to bottom")}
		tooltip={t("Scroll to bottom")}
		size="lg"
		iconSize={ICON_CLASS_DEFAULT}
		class="h-9 w-9 rounded-full bg-accent text-accent-foreground absolute bottom-4 shadow-md"
	/>
</div>
