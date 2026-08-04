<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { presetsStore } from '$lib/stores/presets.svelte';
	import * as Collapsible from '$lib/components/ui/collapsible';
	import { Button } from '$lib/components/ui/button';
	import { Textarea } from '$lib/components/ui/textarea';
	import { Input } from '$lib/components/ui/input';
	import { TruncatedText } from '$lib/components/app';
	import * as Tooltip from '$lib/components/ui/tooltip';
	import {
		Star,
		Trash2,
		Plus,
		Pencil,
		Check,
		ChevronDown,
		ChevronRight,
		Search,
		X
	} from '@lucide/svelte';
	import DialogPresetWizard from '$lib/components/app/dialogs/DialogPresetWizard.svelte';

	let expanded = $state(false);
	let showWizard = $state(false);
	let searchQuery = $state('');

	// Row edit buffers: id -> { name, description, content }
	let editBuffers = $state<Record<string, { name: string; description: string; content: string }>>({});

	const filtered = $derived(
		presetsStore.presets.filter((preset) => {
			const q = searchQuery.trim().toLowerCase();
			if (!q) return true;
			return (
				preset.name.toLowerCase().includes(q) || (preset.description ?? '').toLowerCase().includes(q)
			);
		})
	);

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

<Collapsible.Root open={expanded} onOpenChange={() => (expanded = !expanded)}>
	<Collapsible.Trigger class="flex w-full items-center gap-2 rounded-lg px-3 py-2 text-sm hover:bg-muted/50">
		{#if expanded}
			<ChevronDown class="h-3.5 w-3.5 shrink-0" />
		{:else}
			<ChevronRight class="h-3.5 w-3.5 shrink-0" />
		{/if}

		<span class="inline-flex min-w-0 items-center gap-1.5 font-medium">
			<TruncatedText text={t('Custom prompt presets')} class="font-medium" />
		</span>

		<span class="ml-auto shrink-0 text-xs text-muted-foreground">{presetsStore.presets.length}</span>

		<Button
			type="button"
			variant="ghost"
			size="sm"
			class="shrink-0"
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
		{#if expanded}
			<div class="ml-4 border-l border-border/50 pl-2">
				<div class="relative px-2 py-1.5">
					<Search class="absolute top-1/2 left-5 z-10 h-3.5 w-3.5 -translate-y-1/2 text-muted-foreground" />
					<Input class="h-8 pl-8" placeholder={t('Search presets...')} value={searchQuery} oninput={(e) => (searchQuery = e.currentTarget.value)} />
					{#if searchQuery}
						<button
							type="button"
							class="absolute top-1/2 right-4 z-10 -translate-y-1/2 text-muted-foreground hover:text-foreground"
							title={t('Clear')}
							onclick={() => (searchQuery = '')}
						>
							<X class="h-3.5 w-3.5" />
						</button>
					{/if}
				</div>

				<!-- Header row -->
				<div class="flex items-center gap-3 px-2 py-1 text-xs text-muted-foreground">
					<span class="min-w-0 flex-1">{t('Preset')}</span>
					<span class="w-20 shrink-0"></span>
					<span class="w-20 shrink-0"></span>
					<span class="w-20 shrink-0"></span>
				</div>

				{#if filtered.length === 0}
					<div class="px-2 py-4 text-center text-sm text-muted-foreground">
						{presetsStore.presets.length === 0 ? t('No presets yet') : t('No matches')}
					</div>
				{:else}
					{#each filtered as preset (preset.id)}
						{@const buffer = bufferFor(preset.id)}
						{@const editing = !!editBuffers[preset.id]}
						<div>
							<div class="flex items-center gap-3 rounded px-2 py-1.5 text-sm hover:bg-muted/50">
								<span class="flex min-w-0 flex-1 items-center gap-1.5">
									<Tooltip.Root delayDuration={300}>
										<Tooltip.Trigger>
											{#snippet child({ props })}
												<span {...props} class="flex min-w-0 flex-1 cursor-pointer">
													<TruncatedText text={preset.name} class="min-w-0 font-medium" />
												</span>
											{/snippet}
										</Tooltip.Trigger>
										<Tooltip.Content side="top">
											<p>{preset.description ?? preset.name}</p>
										</Tooltip.Content>
									</Tooltip.Root>
								</span>

								<div class="flex w-20 shrink-0 justify-center">
									<Tooltip.Root delayDuration={300}>
										<Tooltip.Trigger>
											{#snippet child({ props })}
												<button
													type="button"
													{...props}
													class="shrink-0 p-1 text-muted-foreground hover:text-foreground"
													onclick={() => presetsStore.toggleFavorite(preset.id)}
												>
													<Star class="h-4 w-4 {preset.favorite ? 'fill-amber-400 text-amber-400' : ''}" />
												</button>
											{/snippet}
										</Tooltip.Trigger>
										<Tooltip.Content side="top">
											<p>{preset.favorite ? t('Remove from favorites') : t('Add to favorites')}</p>
										</Tooltip.Content>
									</Tooltip.Root>
								</div>

								<div class="flex w-20 shrink-0 justify-center">
									{#if !editing}
										<Tooltip.Root delayDuration={300}>
											<Tooltip.Trigger>
												{#snippet child({ props })}
													<button
														type="button"
														{...props}
														class="shrink-0 p-1 text-muted-foreground hover:text-foreground"
														onclick={() => (editBuffers[preset.id] = bufferFor(preset.id))}
													>
														<Pencil class="h-4 w-4" />
													</button>
												{/snippet}
											</Tooltip.Trigger>
											<Tooltip.Content side="top">
												<p>{t('Edit')}</p>
											</Tooltip.Content>
										</Tooltip.Root>
									{/if}
								</div>

								<div class="flex w-20 shrink-0 justify-center">
									<Tooltip.Root delayDuration={300}>
										<Tooltip.Trigger>
											{#snippet child({ props })}
												<button
													type="button"
													{...props}
													class="shrink-0 p-1 text-muted-foreground hover:text-destructive"
													onclick={() => presetsStore.remove(preset.id)}
												>
													<Trash2 class="h-4 w-4" />
												</button>
											{/snippet}
										</Tooltip.Trigger>
										<Tooltip.Content side="top">
											<p>{t('Delete')}</p>
										</Tooltip.Content>
									</Tooltip.Root>
								</div>
							</div>

							{#if editing}
								<div class="space-y-2 border-t border-border/40 px-3 py-3">
									<Input bind:value={buffer.name} placeholder={t('Preset name')} />
									<Textarea
										bind:value={buffer.description}
										rows={2}
										placeholder={t('Short description (shown in the picker)')}
									/>
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
		{/if}
	</Collapsible.Content>
</Collapsible.Root>

<DialogPresetWizard bind:open={showWizard} />
