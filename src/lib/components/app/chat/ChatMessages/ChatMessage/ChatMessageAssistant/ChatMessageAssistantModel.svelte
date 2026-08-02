<script>import { copyToClipboard } from '$lib/utils';
let { displayedModel, isRouter, isLoading, onRegenerate } = $props();
let pendingModel = $state(null);
function handleCopyModel() {
    void copyToClipboard(displayedModel ?? '');
}
</script>

{#if isRouter}
	<ModelsSelectorDropdown
		currentModel={pendingModel ?? displayedModel}
		disabled={isLoading}
		onModelChange={async (modelId, modelName) => {
			const status = modelsStore.getModelStatus(modelId);

			if (status !== ServerModelStatus.LOADED) {
				pendingModel = modelId;

				try {
					await modelsStore.loadModel(modelId);
				} finally {
					pendingModel = null;
				}
			}

			onRegenerate(modelName);
			return true;
		}}
	/>
{:else}
	<ModelBadge model={displayedModel || undefined} onclick={handleCopyModel} />
{/if}
