<script lang="ts">
	import { cn, WithElementRef } from '$lib/components/ui/utils.js';
	import { HTMLAttributes } from 'svelte/elements';

	let {
		ref = $bindable(null),
		class: className,
		children,
		...restProps
	}: WithElementRef<HTMLAttributes<HTMLTableSectionElement>> = $props();
</script>

<thead
	bind:this={ref}
	data-slot="table-header"
	class={cn('[&_tr]:border-b', className)}
	{...restProps}
>
	{@render children?.()}
</thead>
