<script>let { argument, value = '', suggestions = [], isLoadingSuggestions = false, isAutocompleteActive = false, autocompleteIndex = 0, onInput, onKeydown, onBlur, onFocus, onSelectSuggestion } = $props();
export {};
</script>

<div class="relative grid gap-1">
	<Label for="arg-{argument.name}" class="mb-1 text-muted-foreground">
		<span>
			{argument.name}

			{#if argument.required}
				<span class="text-destructive">*</span>
			{/if}
		</span>

		{#if isLoadingSuggestions}
			<span class="text-xs text-muted-foreground/50">...</span>
		{/if}
	</Label>

	<Input
		id="arg-{argument.name}"
		type="text"
		{value}
		oninput={(e) => onInput(e.currentTarget.value)}
		onkeydown={onKeydown}
		onblur={onBlur}
		onfocus={onFocus}
		placeholder={argument.description || argument.name}
		required={argument.required}
		autocomplete="off"
	/>

	{#if isAutocompleteActive && suggestions.length > 0}
		<div
			class="absolute top-full right-0 left-0 z-10 mt-1 max-h-32 overflow-y-auto rounded-lg border border-border/50 bg-background shadow-lg"
			transition:fly={{ y: -5, duration: 100 }}
		>
			{#each suggestions as suggestion, i (suggestion)}
				<button
					type="button"
					onmousedown={() => onSelectSuggestion(suggestion)}
					class="w-full px-3 py-1.5 text-left text-sm hover:bg-accent {i === autocompleteIndex
						? 'bg-accent'
						: ''}"
				>
					{suggestion}
				</button>
			{/each}
		</div>
	{/if}
</div>
