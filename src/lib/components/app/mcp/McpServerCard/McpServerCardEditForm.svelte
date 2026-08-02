<script>import { parseHeadersToArray } from '$lib/utils';
let { serverId, serverUrl, serverUseProxy = false, serverLabel = '', onSave, onCancel } = $props();
let editUrl = $derived(serverUrl);
let editName = $derived(serverLabel);
let editHeaders = $state('');
let editUseProxy = $derived(serverUseProxy);
let urlError = $derived.by(() => {
    if (!editUrl.trim())
        return 'URL is required';
    try {
        new URL(editUrl);
        return null;
    }
    catch {
        return 'Invalid URL format';
    }
});
let headerPairsValid = $derived(parseHeadersToArray(editHeaders).every((p) => p.key.trim() && p.value.trim()));
let canSave = $derived(!urlError && headerPairsValid);
function handleSave() {
    if (!canSave)
        return;
    // An unchanged prefill keeps following the automatic label; only an
    // actual edit becomes a persisted custom display name.
    const name = editName.trim() !== serverLabel.trim() ? editName.trim() : undefined;
    onSave(editUrl.trim(), editHeaders.trim(), editUseProxy, name);
}
function handleSubmit(event) {
    event.preventDefault();
    handleSave();
}
export function setInitialValues(url, headers, useProxy, name = '') {
    editUrl = url;
    editHeaders = headers;
    editUseProxy = useProxy;
    editName = name;
}
</script>

<form onsubmit={handleSubmit} class="contents">
	<div class="space-y-4">
		<p class="font-medium">{t("Configure Server")}</p>

		<McpServerForm
			url={editUrl}
			name={editName}
			onNameChange={(v) => (editName = v)}
			headers={editHeaders}
			useProxy={editUseProxy}
			onUrlChange={(v) => (editUrl = v)}
			onHeadersChange={(v) => (editHeaders = v)}
			onUseProxyChange={(v) => (editUseProxy = v)}
			urlError={editUrl ? urlError : null}
			id={serverId}
		/>

		<div class="flex items-center justify-end gap-2">
			<Button variant="secondary" size="sm" onclick={onCancel}>{t("Cancel")}</Button>

			<Button size="sm" type="submit" disabled={!canSave}>
				{serverUrl.trim() ? t("Update") : t("Add")}
			</Button>
		</div>
	</div>
</form>
