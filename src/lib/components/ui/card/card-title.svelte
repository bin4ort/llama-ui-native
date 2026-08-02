<script lang="ts">
	import { HTMLAttributes } from 'svelte/elements';
	import { cn, WithElementRef } from '$lib/components/ui/utils';

	let {
		ref = $bindable(null),
		class: className,
		children,
		...restProps
	}: WithElementRef<HTMLAttributes<HTMLDivElement>> = $props();
</script>

<div
	bind:this={ref}
	data-slot="card-title"
	class={cn('leading-none font-semibold', className)}
	{...restProps}
>
	{@render children?.()}
</div>
