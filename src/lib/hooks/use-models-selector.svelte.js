import { onMount } from 'svelte';
import { modelsStore, modelOptions, modelsLoading, modelsUpdating, selectedModelId, singleModelName } from '$lib/stores/models.svelte';
import { isRouterMode } from '$lib/stores/server.svelte';
import { filterModelOptions, groupModelOptions } from '$lib/components/app/models/utils';
/**
 * Shared reactive state and logic for model selection.
 *
 * Used by both the desktop dropdown (`ModelsSelectorDropdown`)
 * and the mobile sheet (`ModelsSelectorSheet`) to avoid
 * duplicating store derivations, selection handling, and model loading.
 */
export function useModelsSelector(opts) {
    const options = $derived(modelOptions().filter((option) => {
        const modelProps = modelsStore.getModelProps(option.model);
        return modelProps?.ui !== false;
    }));
    const loading = $derived(modelsLoading());
    const updating = $derived(modelsUpdating());
    const activeId = $derived(selectedModelId());
    const isRouter = $derived(isRouterMode());
    const serverModel = $derived(singleModelName());
    const currentModel = $derived(opts.currentModel());
    const onModelChange = $derived(opts.onModelChange?.());
    const isHighlightedCurrentModelActive = $derived.by(() => {
        if (!isRouter || !currentModel)
            return false;
        const currentOption = options.find((option) => option.model === currentModel);
        return currentOption ? currentOption.id === activeId : false;
    });
    const isCurrentModelInCache = $derived.by(() => {
        if (!isRouter || !currentModel)
            return true;
        return options.some((option) => option.model === currentModel);
    });
    let isLoadingModel = $state(false);
    let searchTerm = $state('');
    let showModelDialog = $state(false);
    let infoModelId = $state(null);
    const filteredOptions = $derived(filterModelOptions(options, searchTerm));
    const groupedFilteredOptions = $derived(groupModelOptions(filteredOptions, modelsStore.favoriteModelIds, (m) => modelsStore.isModelLoaded(m)));
    function handleInfoClick(modelName) {
        infoModelId = modelName;
        showModelDialog = true;
    }
    onMount(() => {
        modelsStore.fetch().catch((error) => {
            console.error('Unable to load models:', error);
        });
    });
    function handleOpenChange(open) {
        if (loading || updating)
            return;
        if (isRouter) {
            searchTerm = '';
            if (open) {
                modelsStore.fetchRouterModels().then(() => {
                    modelsStore.fetchModalitiesForLoadedModels();
                });
            }
            opts.onOpenChange?.(open);
        }
        else {
            showModelDialog = open;
        }
    }
    async function handleSelect(modelId) {
        const option = options.find((opt) => opt.id === modelId);
        if (!option)
            return;
        let shouldCloseMenu = true;
        if (onModelChange) {
            const result = await onModelChange(option.id, option.model);
            if (result === false) {
                shouldCloseMenu = false;
            }
        }
        else {
            await modelsStore.selectModelById(option.id);
        }
        if (shouldCloseMenu) {
            handleOpenChange(false);
            requestAnimationFrame(() => {
                const textarea = document.querySelector('[data-slot="chat-form"] textarea');
                textarea?.focus({ preventScroll: true });
            });
        }
        if (!onModelChange && isRouter && !modelsStore.isModelLoaded(option.model)) {
            isLoadingModel = true;
            modelsStore
                .loadModel(option.model)
                .catch((error) => console.error('Failed to load model:', error))
                .finally(() => (isLoadingModel = false));
        }
    }
    function getDisplayOption() {
        if (!isRouter) {
            const displayModel = serverModel || currentModel;
            if (displayModel) {
                return {
                    id: serverModel ? 'current' : 'offline-current',
                    model: displayModel,
                    name: displayModel.split('/').pop() || displayModel,
                    capabilities: []
                };
            }
            return undefined;
        }
        if (currentModel) {
            if (!isCurrentModelInCache) {
                return {
                    id: 'not-in-cache',
                    model: currentModel,
                    name: currentModel.split('/').pop() || currentModel,
                    capabilities: []
                };
            }
            return options.find((option) => option.model === currentModel);
        }
        if (activeId) {
            return options.find((option) => option.id === activeId);
        }
        return undefined;
    }
    return {
        get options() {
            return options;
        },
        get loading() {
            return loading;
        },
        get updating() {
            return updating;
        },
        get activeId() {
            return activeId;
        },
        get isRouter() {
            return isRouter;
        },
        get serverModel() {
            return serverModel;
        },
        get isHighlightedCurrentModelActive() {
            return isHighlightedCurrentModelActive;
        },
        get isCurrentModelInCache() {
            return isCurrentModelInCache;
        },
        get filteredOptions() {
            return filteredOptions;
        },
        get groupedFilteredOptions() {
            return groupedFilteredOptions;
        },
        get isLoadingModel() {
            return isLoadingModel;
        },
        get searchTerm() {
            return searchTerm;
        },
        get showModelDialog() {
            return showModelDialog;
        },
        get infoModelId() {
            return infoModelId;
        },
        setSearchTerm(value) {
            searchTerm = value;
        },
        setShowModelDialog(value) {
            showModelDialog = value;
        },
        handleInfoClick,
        handleSelect,
        handleOpenChange,
        isFavorite(model) {
            return modelsStore.favoriteModelIds.has(model);
        },
        getDisplayOption
    };
}
