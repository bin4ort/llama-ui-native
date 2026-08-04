<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { settingsStore } from '$lib/stores/settings.svelte';
	import { SETTINGS_KEYS } from '$lib/constants';
	import { BUILTIN_TOOLS } from '$lib/constants/builtin-tools';
	import * as Collapsible from '$lib/components/ui/collapsible';
	import { Checkbox } from '$lib/components/ui/checkbox';
	import { Label } from '$lib/components/ui/label';
	import { ChevronDown, ChevronRight } from '@lucide/svelte';

	let expanded = $state(false);

	const enabledCount = $derived(
		BUILTIN_TOOLS.filter((meta) => settingsStore.config[meta.configKey as keyof typeof settingsStore.config] !== false)
			.length
	);

	function toggle(configKey: string, checked: boolean) {
		settingsStore.updateConfig(configKey as keyof typeof settingsStore.config, checked);
	}
</script>

<div class="space-y-2 rounded-lg border border-border/50 bg-muted/20 p-3">
	<Collapsible.Root bind:open={expanded}>
		<Collapsible.Trigger class="flex w-full items-center justify-between gap-2 text-left">
			<div class="flex items-center gap-2 text-sm font-medium">
				{#if expanded}
					<ChevronDown class="h-4 w-4" />
				{:else}
					<ChevronRight class="h-4 w-4" />
				{/if}
				<span>{t('Built-in tools')}</span>
				<span class="text-xs text-muted-foreground">({enabledCount}/{BUILTIN_TOOLS.length})</span>
			</div>
		</Collapsible.Trigger>

		<Collapsible.Content>
			<div class="mt-3 space-y-2">
				{#each BUILTIN_TOOLS as tool (tool.name)}
					{@const isEnabled =
						settingsStore.config[tool.configKey as keyof typeof settingsStore.config] !== false}
					<div class="flex items-start space-x-3 rounded-md px-1 py-0.5">
						<Checkbox
							id={tool.configKey}
							checked={isEnabled}
							onCheckedChange={(checked) => toggle(tool.configKey, Boolean(checked))}
							class="mt-1"
						/>
						<label
							for={tool.configKey}
							class="flex cursor-pointer items-center gap-1.5 pt-1 pb-0.5 text-sm leading-none font-medium"
						>
							{t(tool.labelKey)}
						</label>
					</div>
				{/each}

				<p class="pt-1 text-xs text-muted-foreground">
					{t('Enabled tools are offered to the model in agentic flows; permission is still asked per call.')}
				</p>
			</div>
		</Collapsible.Content>
	</Collapsible.Root>
</div>
