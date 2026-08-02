<script>let { isSelected = false, onclick, dataIndex, children } = $props();
export {};
</script>

<button
	type="button"
	data-picker-index={dataIndex}
	{onclick}
	class="flex w-full cursor-pointer items-start gap-3 rounded-lg px-3 py-2 text-left hover:bg-accent/50 {isSelected
		? 'bg-accent/50'
		: ''}"
>
	{@render children()}
</button>
