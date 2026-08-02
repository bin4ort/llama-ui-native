<script>let { class: className = '', height = 'h-16', id, imageClass = '', onclick, onRemove, name, preview, readonly = false, width = 'w-auto' } = $props();
export {};
</script>

{#snippet image()}
	<img src={preview} alt={name} class="{height} {width} cursor-pointer object-cover {imageClass}" />
{/snippet}

<div
	class="group relative overflow-hidden rounded-lg bg-muted shadow-lg dark:border dark:border-muted {className}"
>
	{#if onclick}
		<button
			aria-label="Preview {name}"
			class="block h-full w-full rounded-lg focus:ring-2 focus:ring-primary focus:ring-offset-2 focus:outline-none"
			{onclick}
			type="button"
		>
			{@render image()}
		</button>
	{:else}
		{@render image()}
	{/if}

	{#if !readonly}
		<div
			class="absolute top-1 right-1 flex items-center justify-center opacity-0 transition-opacity group-focus-within:opacity-100 group-hover:opacity-100"
		>
			<ActionIcon
				class="text-white"
				icon={X}
				onclick={() => onRemove?.(id)}
				stopPropagationOnClick
				tooltip="Remove"
			/>
		</div>
	{/if}
</div>
