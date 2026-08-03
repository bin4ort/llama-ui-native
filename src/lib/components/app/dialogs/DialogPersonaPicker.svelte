<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { presetsStore } from '$lib/stores/presets.svelte';
	import { conversationsStore } from '$lib/stores/conversations.svelte';
	import { chatStore } from '$lib/stores/chat.svelte';
	import { PRESET_DEFAULT_ID } from '$lib/constants/presets';
	import type { SystemPromptPreset } from '$lib/types';
	import * as Dialog from '$lib/components/ui/dialog';
	import { Button } from '$lib/components/ui/button';
	import { ScrollArea } from '$lib/components/ui/scroll-area';
	import { SearchInput } from '$lib/components/app/forms';
	import { Star, Trash2, Plus, Check } from '@lucide/svelte';
	import DialogPresetWizard from './DialogPresetWizard.svelte';

	interface Props {
		open?: boolean;
		onOpenChange?: (open: boolean) => void;
	}

	let { open = $bindable(), onOpenChange }: Props = $props();

	let searchQuery = $state('');
	let showWizard = $state(false);

	// Effective current persona: exact content match against the library,
	// otherwise the default.
	const activePresetId = $derived.by(() => {
		const am = conversationsStore.activeMessages;
		const sys = am.find((m) => m.role === 'system');
		const content = typeof sys?.content === 'string' ? sys.content.trim() : '';
		if (!content) return PRESET_DEFAULT_ID;
		return presetsStore.presets.find((p) => p.content.trim() === content)?.id ?? PRESET_DEFAULT_ID;
	});

	const filtered = $derived(
		presetsStore.presets.filter((p) => {
			const q = searchQuery.trim().toLowerCase();
			if (!q) return true;
			return (
				p.name.toLowerCase().includes(q) || (p.description ?? '').toLowerCase().includes(q)
			);
		})
	);

	function applyPreset(preset: SystemPromptPreset | null) {
		const conv = conversationsStore.activeConversation;
		if (!conv) return;
		void chatStore.applySystemPromptContent(conv.id, preset ? preset.content : '');
		open = false;
	}

	function applyDefault() {
		const conv = conversationsStore.activeConversation;
		if (!conv) return;
		void chatStore.applySystemPromptContent(conv.id, '');
		open = false;
	}
</script>

<Dialog.Root bind:open onOpenChange={(o) => { if (!o) { searchQuery = ''; showWizard = false; } onOpenChange?.(o); }}>
	<Dialog.Portal>
		<Dialog.Overlay class="z-9999" />
		<Dialog.Content class="z-9999 max-w-lg">
			<Dialog.Header>
				<Dialog.Title>{t('Personality presets')}</Dialog.Title>
				<Dialog.Description>{t('Pick a personality for this conversation. It applies from the next message.')}</Dialog.Description>
			</Dialog.Header>

			<div class="space-y-3 py-2">
				<SearchInput bind:value={searchQuery} placeholder={t('Search presets...')} />

				{#if !showWizard}
					<ScrollArea class="h-72 pr-3">
						<div class="space-y-1">
							<button
								type="button"
								class="flex w-full items-center justify-between gap-2 rounded-md px-3 py-2 text-left text-sm transition-colors hover:bg-accent {activePresetId === PRESET_DEFAULT_ID
									? 'bg-accent text-accent-foreground'
									: ''}"
								onclick={applyDefault}
							>
								<span>{t('Default')}</span>
								{#if activePresetId === PRESET_DEFAULT_ID}
									<Check class="h-4 w-4 shrink-0" />
								{/if}
							</button>

							{#each filtered as preset (preset.id)}
								{@const isActive = activePresetId === preset.id}
								<div class="group flex items-center gap-1 rounded-md transition-colors hover:bg-accent {isActive
									? 'bg-accent text-accent-foreground'
									: ''}">
									<button
										type="button"
										class="flex min-w-0 flex-1 items-center gap-2 px-3 py-2 text-left text-sm"
										onclick={() => applyPreset(preset)}
									>
										<span class="min-w-0 flex-1 truncate">{preset.name}</span>
										{#if preset.description}
											<span class="hidden min-w-0 flex-1 truncate text-xs text-muted-foreground sm:block"
												>{preset.description}</span
											>
										{/if}
										{#if isActive}
											<Check class="h-4 w-4 shrink-0" />
										{/if}
									</button>

									<button
										type="button"
										class="shrink-0 p-2 text-muted-foreground hover:text-foreground"
										title={preset.favorite ? t('Remove from favorites') : t('Add to favorites')}
										onclick={() => presetsStore.toggleFavorite(preset.id)}
									>
										<Star class="h-4 w-4 {preset.favorite ? 'fill-amber-400 text-amber-400' : ''}" />
									</button>

									<button
										type="button"
										class="shrink-0 p-2 text-muted-foreground opacity-0 transition-opacity hover:text-destructive group-hover:opacity-100"
										title={t('Delete')}
										onclick={() => presetsStore.remove(preset.id)}
									>
										<Trash2 class="h-4 w-4" />
									</button>
								</div>
							{:else}
								{#if searchQuery}
									<p class="px-3 py-4 text-center text-sm text-muted-foreground">{t('No presets found')}</p>
								{:else}
									<p class="px-3 py-4 text-center text-sm text-muted-foreground">{t('No presets yet')}</p>
								{/if}
							{/each}
						</div>
					</ScrollArea>
				{/if}

				<DialogPresetWizard
					bind:open={showWizard}
					onSaved={() => {
						showWizard = false;
					}}
				/>
			</div>

			<Dialog.Footer>
				<Button
					variant="outline"
					onclick={() => {
						showWizard = !showWizard;
					}}
				>
					<Plus class="mr-2 h-4 w-4" />
					{t('Create preset')}
				</Button>
				<Button variant="ghost" onclick={() => (open = false)}>{t('Close')}</Button>
			</Dialog.Footer>
		</Dialog.Content>
	</Dialog.Portal>
</Dialog.Root>
