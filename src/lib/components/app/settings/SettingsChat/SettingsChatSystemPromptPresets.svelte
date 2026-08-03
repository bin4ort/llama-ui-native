<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { presetsStore } from '$lib/stores/presets.svelte';
	import * as Collapsible from '$lib/components/ui/collapsible';
	import { Button } from '$lib/components/ui/button';
	import { Textarea } from '$lib/components/ui/textarea';
	import { Input } from '$lib/components/ui/input';
	import { Star, Trash2, Plus, ChevronDown, ChevronRight, Check } from '@lucide/svelte';
	import DialogPresetWizard from '$lib/components/app/dialogs/DialogPresetWizard.svelte';

	let expanded = $state(false);
	let showWizard = $state(false);

	// Row edit buffers: id -> { name, description, content }
	let editBuffers = $state<Record<string, { name: string; description: string; content: string }>>({});

	function bufferFor(id: string) {
		if (!editBuffers[id]) {
			const p = presetsStore.getById(id);
			editBuffers[id] = {
				name: p?.name ?? '',
				description: p?.description ?? '',
				content: p?.content ?? ''
			};
		}
		return editBuffers[id];
	}

	function savePreset(id: string) {
		const b = editBuffers[id];
		if (!b) return;
		presetsStore.update(id, {
			name: b.name,
			description: b.description,
			content: b.content
		});
		delete editBuffers[id];
	}

	function discardEdit(id: string) {
		delete editBuffers[id];
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
				<span>{t('Custom prompt presets')}</span>
				<span class="text-xs text-muted-foreground">({presetsStore.presets.length})</span>
			</div>

			<Button
				type="button"
				variant="ghost"
				size="sm"
				onclick={(e: MouseEvent) => {
					e.stopPropagation();
					showWizard = true;
				}}
			>
				<Plus class="mr-1 h-3 w-3" />
				{t('Create preset')}
			</Button>
		</Collapsible.Trigger>

		<Collapsible.Content>
			<div class="mt-3 space-y-2">
				{#if presetsStore.presets.length === 0}
					<p class="text-sm text-muted-foreground">{t('No presets yet')}</p>
				{:else}
					{#each presetsStore.presets as preset (preset.id)}
						{@const buffer = bufferFor(preset.id)}
						{@const editing = !!editBuffers[preset.id]}
						<div class="rounded-md border border-border/40 bg-background p-2">
							<div class="flex items-center gap-2">
								<button
									type="button"
									class="shrink-0 p-1 text-muted-foreground hover:text-foreground"
									title={preset.favorite ? t('Remove from favorites') : t('Add to favorites')}
									onclick={() => presetsStore.toggleFavorite(preset.id)}
								>
									<Star class="h-4 w-4 {preset.favorite ? 'fill-amber-400 text-amber-400' : ''}" />
								</button>

								<span class="min-w-0 flex-1 truncate text-sm font-medium">{preset.name}</span>

								{#if !editing}
									<Button type="button" variant="ghost" size="sm" onclick={() => (editBuffers[preset.id] = bufferFor(preset.id))}>
										{t('Edit')}
									</Button>
									<Button
										type="button"
										variant="ghost"
										size="sm"
										class="text-destructive hover:text-destructive"
										onclick={() => presetsStore.remove(preset.id)}
									>
										<Trash2 class="h-3.5 w-3.5" />
									</Button>
								{/if}
							</div>

							{#if editing}
								<div class="mt-2 space-y-2">
									<Input bind:value={buffer.name} placeholder={t('Preset name')} />
									<Input bind:value={buffer.description} placeholder={t('Short description (shown in the picker)')} />
									<Textarea bind:value={buffer.content} rows={4} class="font-mono text-xs" />
									<div class="flex gap-2">
										<Button type="button" size="sm" onclick={() => savePreset(preset.id)}>
											<Check class="mr-1 h-3.5 w-3.5" />
											{t('Save')}
										</Button>
										<Button type="button" variant="ghost" size="sm" onclick={() => discardEdit(preset.id)}>
											{t('Cancel')}
										</Button>
									</div>
								</div>
							{/if}
						</div>
					{/each}
				{/if}
			</div>
		</Collapsible.Content>
	</Collapsible.Root>

	<DialogPresetWizard bind:open={showWizard} />
</div>
