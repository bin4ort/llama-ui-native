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

<tbody
	bind:this={ref}
	data-slot="table-body"
	class={cn('[&_tr:last-child]:border-0', className)}
	{...restProps}
>
	{@render children?.()}
</tbody>
