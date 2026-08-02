<script>import { sanitizeExternalUrl } from '$lib/utils';
let { displayName, faviconUrl = null, serverInfo, iconClass = 'h-5 w-5', iconRounded = 'rounded-sm', showVersion = true, showWebsite = true, nameClass } = $props();
let safeWebsiteUrl = $derived(serverInfo?.websiteUrl ? sanitizeExternalUrl(serverInfo.websiteUrl) : null);
</script>

<span class="flex min-w-0 items-center gap-1.5">
	{#if faviconUrl}
		<img src={faviconUrl} alt="" class={['shrink-0 text-foreground', iconRounded, iconClass]} />
	{:else}
		<McpLogo class={['shrink-0 text-foreground', iconRounded, iconClass].join(' ')} />
	{/if}

	<TruncatedText text={displayName ?? ''} class={nameClass ?? ''} />

	{#if showVersion && serverInfo?.version}
		<Badge variant="secondary" class="h-4 max-w-24 min-w-0 shrink px-1 text-[10px]">
			<TruncatedText text={`v${serverInfo.version}`} />
		</Badge>
	{/if}

	{#if showWebsite && safeWebsiteUrl}
		<a
			href={safeWebsiteUrl}
			target="_blank"
			rel="noopener noreferrer"
			class="shrink-0 text-muted-foreground hover:text-foreground"
			aria-label={t("Open website")}
			onclick={(e) => e.stopPropagation()}
		>
			<ExternalLink class="h-3 w-3" />
		</a>
	{/if}
</span>
