import { SvelteMap } from 'svelte/reactivity';
export function filterModelOptions(options, searchTerm) {
    const term = searchTerm.trim().toLowerCase();
    if (!term)
        return options;
    return options.filter((option) => option.model.toLowerCase().includes(term) ||
        option.name?.toLowerCase().includes(term) ||
        option.aliases?.some((alias) => alias.toLowerCase().includes(term)) ||
        option.tags?.some((tag) => tag.toLowerCase().includes(term)));
}
export function groupModelOptions(filteredOptions, favoriteIds, isModelLoaded) {
    // Loaded models
    const loaded = [];
    for (let i = 0; i < filteredOptions.length; i++) {
        if (isModelLoaded(filteredOptions[i].model)) {
            loaded.push({ option: filteredOptions[i], flatIndex: i });
        }
    }
    // Favorites (excluding loaded)
    const loadedModelIds = new Set(loaded.map((item) => item.option.model));
    const favorites = [];
    for (let i = 0; i < filteredOptions.length; i++) {
        if (favoriteIds.has(filteredOptions[i].model) &&
            !loadedModelIds.has(filteredOptions[i].model)) {
            favorites.push({ option: filteredOptions[i], flatIndex: i });
        }
    }
    // Available models grouped by org (excluding loaded and favorites)
    const available = [];
    const orgGroups = new SvelteMap();
    for (let i = 0; i < filteredOptions.length; i++) {
        const option = filteredOptions[i];
        if (loadedModelIds.has(option.model) || favoriteIds.has(option.model))
            continue;
        const key = option.parsedId?.orgName ?? '';
        if (!orgGroups.has(key))
            orgGroups.set(key, []);
        orgGroups.get(key).push({ option, flatIndex: i });
    }
    for (const [orgName, items] of orgGroups) {
        available.push({ orgName: orgName || null, items });
    }
    return { loaded, favorites, available };
}
