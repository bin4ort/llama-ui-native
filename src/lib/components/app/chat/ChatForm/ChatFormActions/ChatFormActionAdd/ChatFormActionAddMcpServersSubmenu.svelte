<script>import { conversationsStore } from '$lib/stores/conversations.svelte';
import { mcpStore } from '$lib/stores/mcp.svelte';
import { goto } from '$app/navigation';
import { ROUTES } from '$lib/constants/routes';
let { onMcpSettingsClick } = $props();
let mcpSearchQuery = $state('');
// Every configured server is listed; `enabled` is an on/off state,
// not a visibility filter, so a disabled server stays toggleable.
let mcpServers = $derived(mcpStore.getServers());
let hasMcpServers = $derived(mcpServers.length > 0);
let filteredMcpServers = $derived.by(() => {
    const query = mcpSearchQuery.toLowerCase().trim();
    if (!query)
        return mcpServers;
    return mcpServers.filter((s) => {
        const name = getServerLabel(s).toLowerCase();
        const url = s.url.toLowerCase();
        return name.includes(query) || url.includes(query);
    });
});
function getServerLabel(server) {
    return mcpStore.getServerLabel(server);
}
function isServerEnabledForChat(serverId) {
    return conversationsStore.isMcpServerEnabledForChat(serverId);
}
async function toggleServerForChat(serverId) {
    await conversationsStore.toggleMcpServerForChat(serverId);
}
function handleMcpSubMenuOpen(open) {
    if (open) {
        mcpSearchQuery = '';
        mcpStore.runHealthChecksForServers(mcpServers);
    }
}
function handleMcpSettingsClick() {
    onMcpSettingsClick?.();
    goto(`${hasMcpServers ? '' : '?add'}${ROUTES.MCP_SERVERS}`);
}
</script>

<DropdownMenu.Root>
	<DropdownMenu.Sub onOpenChange={handleMcpSubMenuOpen}>
		<DropdownMenu.SubTrigger class="flex cursor-pointer items-center gap-2">
			<McpLogo class={ICON_CLASS_DEFAULT} />

			<span>{t('MCP Servers')}</span>
		</DropdownMenu.SubTrigger>

		<DropdownMenu.SubContent class="w-72 pt-0">
			{#if hasMcpServers}
				<DropdownMenuSearchable
					placeholder={t("Search servers...")}
					bind:searchValue={mcpSearchQuery}
					emptyMessage={t("No servers found")}
					isEmpty={filteredMcpServers.length === 0}
				>
					<div class="max-h-64 overflow-y-auto">
						{#each filteredMcpServers as server (server.id)}
							{@const healthState = mcpStore.getHealthCheckState(server.id)}
							{@const hasError = healthState.status === HealthCheckStatus.ERROR}
							{@const isEnabledForChat = isServerEnabledForChat(server.id)}
							{@const displayName = getServerLabel(server)}
							{@const faviconUrl = mcpStore.getServerFavicon(server.id)}

							<button
								type="button"
								class="flex w-full items-center justify-between gap-2 rounded-sm px-2 py-2 text-left transition-colors hover:bg-accent disabled:cursor-not-allowed disabled:opacity-50"
								onclick={() => !hasError && toggleServerForChat(server.id)}
								disabled={hasError}
							>
								<div class="flex min-w-0 flex-1 items-center gap-2">
									<div class="min-w-0 flex-1">
										<McpServerIdentity
											{displayName}
											{faviconUrl}
											iconClass={ICON_CLASS_DEFAULT}
											iconRounded="rounded-sm"
											showVersion={false}
											nameClass="text-sm"
										/>
									</div>

									{#if hasError}
										<span
											class="shrink-0 rounded bg-destructive/15 px-1.5 py-0.5 text-xs text-destructive"
										>
											{t("Error")}
										</span>
									{/if}
								</div>

								<Switch
									checked={isEnabledForChat}
									disabled={hasError}
									onclick={(e) => e.stopPropagation()}
									onCheckedChange={() => toggleServerForChat(server.id)}
								/>
							</button>
						{/each}
					</div>

					{#snippet footer()}
						<DropdownMenu.Item
							class="flex cursor-pointer items-center gap-2"
							onclick={handleMcpSettingsClick}
						>
							<Settings class={ICON_CLASS_DEFAULT} />

							<span>{t('Manage MCP Servers')}</span>
						</DropdownMenu.Item>
					{/snippet}
				</DropdownMenuSearchable>
			{:else}
				<div class="px-2 py-3 text-center text-sm text-muted-foreground">
					{t("No MCP servers configured")}
				</div>

				<DropdownMenu.Separator />

				<DropdownMenu.Item
					class="flex cursor-pointer items-center gap-2"
					onclick={handleMcpSettingsClick}
				>
					<Plus class={ICON_CLASS_DEFAULT} />

					<span>{t('Add MCP Servers')}</span>
				</DropdownMenu.Item>
			{/if}
		</DropdownMenu.SubContent>
	</DropdownMenu.Sub>
</DropdownMenu.Root>
