<script lang="ts">
	import { cn, WithElementRef } from '$lib/components/ui/utils.js';
	import { HTMLAttributes } from 'svelte/elements';

	let {
		ref = $bindable(null),
		class: className,
		children,
		...restProps
	}: WithElementRef<HTMLAttributes<HTMLElement>> = $props();
</script>

<caption
	bind:this={ref}
	data-slot="table-caption"
	class={cn('mt-4 text-sm text-muted-foreground', className)}
	{...restProps}
>
	{@render children?.()}
</caption>
