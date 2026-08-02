<script>import { t } from '$lib/stores/i18n.svelte';
import { parseHeadersToArray, serializeHeaders } from '$lib/utils';
import { UrlProtocol } from '$lib/enums';
import { AUTHORIZATION_HEADER, BEARER_PREFIX, REDACTED_HEADERS } from '$lib/constants';
let { url, headers, name = '', onNameChange, namePlaceholder = t("Name reported by the server"), useProxy = false, onUrlChange, onHeadersChange, onUseProxyChange, urlError = null, id = 'server', wantsAuthorization = $bindable(false), required = false } = $props();
let isWebSocket = $derived(url.toLowerCase().startsWith(UrlProtocol.WEBSOCKET) ||
    url.toLowerCase().startsWith(UrlProtocol.WEBSOCKET_SECURE));
let headerPairs = $derived(parseHeadersToArray(headers));
// Heuristic: this dedicated UI only owns Authorization headers that already
// carry a Bearer scheme. Anything else (e.g. Basic, raw tokens) stays in the
// KV section so the user can still edit those values verbatim.
const matchesAuthorizationKey = (key) => REDACTED_HEADERS.has(key.trim().toLowerCase());
const isBearerScheme = (value) => value.trim().toLowerCase().startsWith(BEARER_PREFIX.toLowerCase());
const ownedByBearerUi = (p) => matchesAuthorizationKey(p.key) && isBearerScheme(p.value);
let hasAuthorization = $derived(headerPairs.some(ownedByBearerUi));
let showAuthorization = $derived(hasAuthorization || wantsAuthorization);
let urlInput = $state(null);
let bearerInput = $state(null);
$effect(() => {
    urlInput?.focus();
});
$effect(() => {
    if (wantsAuthorization && bearerInput) {
        bearerInput.focus();
    }
});
let bearerToken = $derived.by(() => {
    const auth = headerPairs.find(ownedByBearerUi);
    if (!auth)
        return '';
    return auth.value.trim().slice(BEARER_PREFIX.length).trim();
});
$effect(() => {
    if (!headers.trim()) {
        wantsAuthorization = false;
    }
});
function updateHeaderPairs(newPairs) {
    headerPairs = newPairs;
    onHeadersChange(serializeHeaders(newPairs));
}
// The dedicated UI owns the Authorization slot end-to-end when the user
// engages it: any prior Authorization row (Bearer or otherwise) is replaced
// by exactly one { Authorization: "Bearer <token>" } entry. JSON's last-key
// behavior would otherwise pick one arbitrarily, so we strip first.
function updateBearerToken(token) {
    const filtered = headerPairs.filter((p) => !matchesAuthorizationKey(p.key));
    const trimmed = token.trim();
    if (trimmed) {
        filtered.push({ key: AUTHORIZATION_HEADER, value: `${BEARER_PREFIX}${trimmed}` });
    }
    updateHeaderPairs(filtered);
}
function setUseAuthorization(checked) {
    wantsAuthorization = checked;
    if (!checked) {
        // Only drop the entry this UI owns; a non-Bearer Authorization row
        // authored in the KV section must survive a toggle off untouched.
        const filtered = headerPairs.filter((p) => !ownedByBearerUi(p));
        updateHeaderPairs(filtered);
    }
}
</script>

<div class="grid gap-2">
	<div class="mb-4">
		<label for="server-url-{id}" class="mb-2 block text-xs font-medium select-none">
				{t("Server URL")} <span class="text-destructive">*</span>
			</label>

		<Input
			id="server-url-{id}"
			type="url"
			placeholder={MCP_SERVER_URL_PLACEHOLDER}
			value={url}
			oninput={(e) => onUrlChange(e.currentTarget.value)}
			class={urlError ? 'border-destructive' : ''}
			bind:ref={urlInput}
		/>

		{#if urlError}
			<p class="mt-1.5 text-xs text-destructive">{urlError}</p>
		{/if}
	</div>

	<div class="mb-4">
		<label for="server-name-{id}" class="mb-2 block text-xs font-medium select-none">
				{t("Display name")}
			</label>

		<Input
			id="server-name-{id}"
			type="text"
			placeholder={namePlaceholder}
			value={name}
			oninput={(e) => onNameChange?.(e.currentTarget.value)}
		/>
	</div>

	<label class="flex items-center gap-2 cursor-pointer select-none">
		<Switch
			id="use-authorization-{id}"
			checked={showAuthorization}
			onCheckedChange={setUseAuthorization}
			disabled={required}
		/>

		<span class="text-xs text-muted-foreground">
				{t("Authorization")}{#if required}
				<span class="text-destructive">*</span>{/if}
		</span>
	</label>

	{#if showAuthorization}
		<div class="relative mt-2">
			<Input
				id="bearer-token-{id}"
				type="password"
				autocomplete="off"
				placeholder={t("Paste token here")}
				value={bearerToken}
				oninput={(e) => updateBearerToken(e.currentTarget.value)}
				class="pl-16"
				bind:ref={bearerInput}
			/>

			<span
				class="pointer-events-none absolute inset-y-0 left-3 flex items-center text-sm font-medium text-foreground"
			>
				{t("Bearer")}
			</span>
		</div>
	{/if}

	<KeyValuePairs
		class="mt-3"
		pairs={headerPairs.filter((p) => !ownedByBearerUi(p))}
		onPairsChange={(pairs) => {
			const auth = headerPairs.find(ownedByBearerUi);
			updateHeaderPairs(auth ? [...pairs, auth] : pairs);
		}}
		keyPlaceholder={t("Header name")}
		valuePlaceholder={t("Value")}
		addButtonLabel={t("Add")}
		emptyMessage={t("No custom headers configured.")}
		sectionLabel={t("Custom Headers")}
		sectionLabelOptional
	/>

	{#if !isWebSocket && onUseProxyChange}
		<label
			class={[
				'mt-3 flex items-start gap-2',
				mcpStore.isProxyAvailable && 'cursor-pointer',
				!mcpStore.isProxyAvailable && 'opacity-80'
			]}
		>
			<Switch
				class="mt-1"
				id="use-proxy-{id}"
				checked={useProxy}
				disabled={!mcpStore.isProxyAvailable}
				onCheckedChange={(checked) => onUseProxyChange?.(checked)}
			/>

			<span>
				<span class="text-xs text-muted-foreground">{t("Use llama-server proxy")}</span>

				<br />

				{#if !mcpStore.isProxyAvailable}
					<span class="inline-flex gap-0.75 text-xs text-muted-foreground/60"
						>({t("Run")} <pre>llama-server</pre>
						{t("with")}
						<pre>{CLI_FLAGS.MCP_PROXY}</pre>
						{t("flag")})</span
					>
				{/if}
			</span>
		</label>
	{/if}
</div>
