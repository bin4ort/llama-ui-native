<script>let { displayName, faviconUrl, enabled, disabled = false, onToggle, serverInfo, capabilities, transportType } = $props();
export {};
</script>

<div class="space-y-3">
	<div class="flex items-start justify-between gap-3">
		<div class="flex min-w-0 flex-col gap-3">
			<div class="inline-flex items-center gap-2">
				<McpServerIdentity
					{displayName}
					{faviconUrl}
					{serverInfo}
					iconClass="h-5 w-5"
					iconRounded="rounded"
					nameClass="leading-6 font-medium"
				/>
			</div>

			{#if capabilities || transportType}
				<div class="flex flex-wrap items-center gap-1.5">
					{#if transportType}
						{@const TransportIcon = MCP_TRANSPORT_ICONS[transportType]}
						<Badge variant="outline" class="h-5 gap-1 px-1.5 text-[10px]">
							{#if TransportIcon}
								<TransportIcon class="h-3 w-3" />
							{/if}

							{MCP_TRANSPORT_LABELS[transportType] || transportType}
						</Badge>
					{/if}

					{#if capabilities}
						<McpCapabilitiesBadges {capabilities} />
					{/if}
				</div>
			{/if}
		</div>

		<div class="flex shrink-0 items-center pl-2">
			<Switch checked={enabled} {disabled} onCheckedChange={onToggle} />
		</div>
	</div>
</div>
