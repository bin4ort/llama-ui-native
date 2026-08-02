<script>let { children, class: className = '', icon, ...rest } = $props();
export {};
</script>

<button
	{...rest}
	class={[
		'inline-flex cursor-pointer items-center gap-1 rounded-sm bg-muted-foreground/15 px-1.5 py-0.75',
		className
	]}
>
	{#if icon}
		{@render icon()}
	{/if}

	{@render children()}
</button>
