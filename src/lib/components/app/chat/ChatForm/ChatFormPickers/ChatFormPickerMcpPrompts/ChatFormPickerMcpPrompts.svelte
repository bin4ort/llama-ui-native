<script>import { conversationsStore } from '$lib/stores/conversations.svelte';
import { mcpStore } from '$lib/stores/mcp.svelte';
import { debounce, uuid } from '$lib/utils';
import { KeyboardKey } from '$lib/enums';
import { SvelteMap } from 'svelte/reactivity';
let { class: className = '', isOpen = false, searchQuery = '', onClose, onPromptLoadStart, onPromptLoadComplete, onPromptLoadError } = $props();
let prompts = $state([]);
let isLoading = $state(false);
let selectedPrompt = $state(null);
let promptArgs = $state({});
let selectedIndex = $state(0);
let internalSearchQuery = $state('');
let promptError = $state(null);
let selectedIndexBeforeArgumentForm = $state(null);
let suggestions = $state({});
let loadingSuggestions = $state({});
let activeAutocomplete = $state(null);
let autocompleteIndex = $state(0);
let serverSettingsMap = $derived.by(() => {
    const servers = mcpStore.getServers();
    const map = new SvelteMap();
    for (const server of servers) {
        map.set(server.id, server);
    }
    return map;
});
$effect(() => {
    if (isOpen) {
        loadPrompts();
        selectedIndex = 0;
    }
    else {
        selectedPrompt = null;
        promptArgs = {};
        promptError = null;
    }
});
$effect(() => {
    if (filteredPrompts.length > 0 && selectedIndex >= filteredPrompts.length) {
        selectedIndex = 0;
    }
});
async function loadPrompts() {
    isLoading = true;
    try {
        const perChatOverrides = conversationsStore.getAllMcpServerOverrides();
        const initialized = await mcpStore.ensureInitialized(perChatOverrides);
        if (!initialized) {
            prompts = [];
            return;
        }
        prompts = await mcpStore.getAllPrompts();
    }
    catch (error) {
        console.error('[ChatFormPickerMcpPrompts] Failed to load prompts:', error);
        prompts = [];
    }
    finally {
        isLoading = false;
    }
}
function handlePromptClick(prompt) {
    const args = prompt.arguments ?? [];
    if (args.length > 0) {
        selectedIndexBeforeArgumentForm = selectedIndex;
        selectedPrompt = prompt;
        promptArgs = {};
        promptError = null;
        requestAnimationFrame(() => {
            const firstInput = document.querySelector(`#arg-${args[0].name}`);
            if (firstInput) {
                firstInput.focus();
            }
        });
    }
    else {
        executePrompt(prompt, {});
    }
}
async function executePrompt(prompt, args) {
    promptError = null;
    const placeholderId = uuid();
    const nonEmptyArgs = Object.fromEntries(Object.entries(args).filter(([, value]) => value.trim() !== ''));
    const argsToPass = Object.keys(nonEmptyArgs).length > 0 ? nonEmptyArgs : undefined;
    onPromptLoadStart?.(placeholderId, prompt, argsToPass);
    onClose?.();
    try {
        const result = await mcpStore.getPrompt(prompt.serverName, prompt.name, args);
        onPromptLoadComplete?.(placeholderId, result);
    }
    catch (error) {
        const errorMessage = error instanceof Error ? error.message : 'Unknown error executing prompt';
        onPromptLoadError?.(placeholderId, errorMessage);
    }
}
function handleArgumentSubmit(event) {
    event.preventDefault();
    if (selectedPrompt) {
        executePrompt(selectedPrompt, promptArgs);
    }
}
const fetchCompletions = debounce(async (argName, value) => {
    if (!selectedPrompt || value.length < 1) {
        suggestions[argName] = [];
        return;
    }
    if (import.meta.env.DEV && import.meta.env.VITE_DEBUG) {
        console.log('[ChatFormPickerMcpPrompts] Fetching completions for:', {
            serverName: selectedPrompt.serverName,
            promptName: selectedPrompt.name,
            argName,
            value
        });
    }
    loadingSuggestions[argName] = true;
    try {
        const result = await mcpStore.getPromptCompletions(selectedPrompt.serverName, selectedPrompt.name, argName, value);
        if (import.meta.env.DEV && import.meta.env.VITE_DEBUG) {
            console.log('[ChatFormPickerMcpPrompts] Autocomplete result:', {
                argName,
                value,
                result,
                suggestionsCount: result?.values.length ?? 0
            });
        }
        if (result && result.values.length > 0) {
            // Filter out empty strings from suggestions
            const filteredValues = result.values.filter((v) => v.trim() !== '');
            if (filteredValues.length > 0) {
                suggestions[argName] = filteredValues;
                activeAutocomplete = argName;
                autocompleteIndex = 0;
            }
            else {
                suggestions[argName] = [];
            }
        }
        else {
            suggestions[argName] = [];
        }
    }
    catch (error) {
        console.error('[ChatFormPickerMcpPrompts] Failed to fetch completions:', error);
        suggestions[argName] = [];
    }
    finally {
        loadingSuggestions[argName] = false;
    }
}, 200);
function handleArgInput(argName, value) {
    promptArgs[argName] = value;
    fetchCompletions(argName, value);
}
function selectSuggestion(argName, value) {
    promptArgs[argName] = value;
    suggestions[argName] = [];
    activeAutocomplete = null;
}
function handleArgKeydown(event, argName) {
    const argSuggestions = suggestions[argName] ?? [];
    // Handle Escape - return to prompt selection list
    if (event.key === KeyboardKey.ESCAPE) {
        event.preventDefault();
        event.stopPropagation();
        handleCancelArgumentForm();
        return;
    }
    if (argSuggestions.length === 0 || activeAutocomplete !== argName)
        return;
    if (event.key === KeyboardKey.ARROW_DOWN) {
        event.preventDefault();
        autocompleteIndex = Math.min(autocompleteIndex + 1, argSuggestions.length - 1);
    }
    else if (event.key === KeyboardKey.ARROW_UP) {
        event.preventDefault();
        autocompleteIndex = Math.max(autocompleteIndex - 1, 0);
    }
    else if (event.key === KeyboardKey.ENTER && argSuggestions[autocompleteIndex]) {
        event.preventDefault();
        event.stopPropagation();
        selectSuggestion(argName, argSuggestions[autocompleteIndex]);
    }
}
function handleArgBlur(argName) {
    // Delay to allow click on suggestion
    setTimeout(() => {
        if (activeAutocomplete === argName) {
            suggestions[argName] = [];
            activeAutocomplete = null;
        }
    }, 150);
}
function handleArgFocus(argName) {
    if ((suggestions[argName]?.length ?? 0) > 0) {
        activeAutocomplete = argName;
    }
}
function handleCancelArgumentForm() {
    // Restore the previously selected prompt index
    if (selectedIndexBeforeArgumentForm !== null) {
        selectedIndex = selectedIndexBeforeArgumentForm;
        selectedIndexBeforeArgumentForm = null;
    }
    selectedPrompt = null;
    promptArgs = {};
    promptError = null;
}
export function handleKeydown(event) {
    if (!isOpen)
        return false;
    if (event.key === KeyboardKey.ESCAPE) {
        event.preventDefault();
        if (selectedPrompt) {
            // Return to prompt selection list, keeping the selected prompt active
            handleCancelArgumentForm();
        }
        else {
            onClose?.();
        }
        return true;
    }
    if (event.key === KeyboardKey.ARROW_DOWN) {
        event.preventDefault();
        if (filteredPrompts.length > 0) {
            selectedIndex = (selectedIndex + 1) % filteredPrompts.length;
        }
        return true;
    }
    if (event.key === KeyboardKey.ARROW_UP) {
        event.preventDefault();
        if (filteredPrompts.length > 0) {
            selectedIndex = selectedIndex === 0 ? filteredPrompts.length - 1 : selectedIndex - 1;
        }
        return true;
    }
    if (event.key === KeyboardKey.ENTER && !selectedPrompt) {
        event.preventDefault();
        if (filteredPrompts[selectedIndex]) {
            handlePromptClick(filteredPrompts[selectedIndex]);
        }
        return true;
    }
    return false;
}
let filteredPrompts = $derived.by(() => {
    const sortedServers = mcpStore.getServers();
    const serverOrderMap = new Map(sortedServers.map((server, index) => [server.id, index]));
    const sortedPrompts = [...prompts].sort((a, b) => {
        const orderA = serverOrderMap.get(a.serverName) ?? Number.MAX_SAFE_INTEGER;
        const orderB = serverOrderMap.get(b.serverName) ?? Number.MAX_SAFE_INTEGER;
        return orderA - orderB;
    });
    const query = (searchQuery || internalSearchQuery).toLowerCase();
    if (!query)
        return sortedPrompts;
    return sortedPrompts.filter((prompt) => prompt.name.toLowerCase().includes(query) ||
        prompt.title?.toLowerCase().includes(query) ||
        prompt.description?.toLowerCase().includes(query));
});
let showSearchInput = $derived(prompts.length > 3);
</script>

<ChatFormPickerPopover
	bind:isOpen
	class={className}
	srLabel={t("Open prompt picker")}
	{onClose}
	onKeydown={handleKeydown}
>
	{#if selectedPrompt}
		{@const prompt = selectedPrompt}
		{@const server = serverSettingsMap.get(prompt.serverName)}
		{@const serverLabel = server ? mcpStore.getServerLabel(server) : prompt.serverName}

		<div class="p-4">
			<ChatFormPickerItemHeader
				{server}
				{serverLabel}
				title={prompt.title || prompt.name}
				description={prompt.description}
			>
				{#snippet titleExtra()}
					{#if prompt.arguments?.length}
						<Badge variant="secondary">
							{prompt.arguments.length} arg{prompt.arguments.length > 1 ? 's' : ''}
						</Badge>
					{/if}
				{/snippet}
			</ChatFormPickerItemHeader>

			<ChatFormPromptPickerArgumentForm
				prompt={selectedPrompt}
				{promptArgs}
				{suggestions}
				{loadingSuggestions}
				{activeAutocomplete}
				{autocompleteIndex}
				{promptError}
				onArgInput={handleArgInput}
				onArgKeydown={handleArgKeydown}
				onArgBlur={handleArgBlur}
				onArgFocus={handleArgFocus}
				onSelectSuggestion={selectSuggestion}
				onSubmit={handleArgumentSubmit}
				onCancel={handleCancelArgumentForm}
			/>
		</div>
	{:else}
		<ChatFormPickerList
			items={filteredPrompts}
			{isLoading}
			{selectedIndex}
			bind:searchQuery={internalSearchQuery}
			{showSearchInput}
			searchPlaceholder={t("Search prompts...")}
			emptyMessage={t("No MCP prompts available")}
			itemKey={(prompt) => prompt.serverName + ':' + prompt.name}
		>
			{#snippet item(prompt, index, isSelected)}
				{@const server = serverSettingsMap.get(prompt.serverName)}
				{@const serverLabel = server ? mcpStore.getServerLabel(server) : prompt.serverName}

				<ChatFormPickerListItem
					dataIndex={index}
					{isSelected}
					onclick={() => handlePromptClick(prompt)}
				>
					<ChatFormPickerItemHeader
						{server}
						{serverLabel}
						title={prompt.title || prompt.name}
						description={prompt.description}
					>
						{#snippet titleExtra()}
							{#if prompt.arguments?.length}
								<Badge variant="secondary">
									{prompt.arguments.length} arg{prompt.arguments.length > 1 ? 's' : ''}
								</Badge>
							{/if}
						{/snippet}
					</ChatFormPickerItemHeader>
				</ChatFormPickerListItem>
			{/snippet}

			{#snippet skeleton()}
				<ChatFormPickerListItemSkeleton titleWidth="w-32" showBadge />
			{/snippet}
		</ChatFormPickerList>
	{/if}
</ChatFormPickerPopover>
