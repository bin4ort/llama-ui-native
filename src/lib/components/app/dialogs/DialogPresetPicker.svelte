<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { presetsStore } from '$lib/stores/presets.svelte';
	import { conversationsStore } from '$lib/stores/conversations.svelte';
	import { chatStore } from '$lib/stores/chat.svelte';
	import { PRESET_DEFAULT_ID } from '$lib/constants/presets';
	import type { SystemPromptPreset } from '$lib/types';
	import * as Dialog from '$lib/components/ui/dialog';
	import { Button } from '$lib/components/ui/button';
	import { Input } from '$lib/components/ui/input';
	import { Textarea } from '$lib/components/ui/textarea';
	import { Label } from '$lib/components/ui/label';
	import { ScrollArea } from '$lib/components/ui/scroll-area';
	import { SearchInput } from '$lib/components/app/forms';
	import { Star, Trash2, Plus, Check, Pencil, X } from '@lucide/svelte';
	import DialogPresetWizard from './DialogPresetWizard.svelte';

	interface Props {
		open?: boolean;
		onOpenChange?: (open: boolean) => void;
	}

	let { open = $bindable(), onOpenChange }: Props = $props();

	let searchQuery = $state('');
	let showWizard = $state(false);
	let editingId = $state<string | null>(null);
	let editName = $state('');
	let editDescription = $state('');
	let editContent = $state('');

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

	function startEdit(preset: SystemPromptPreset) {
		editingId = preset.id;
		editName = preset.name;
		editDescription = preset.description ?? '';
		editContent = preset.content;
	}

	function cancelEdit() {
		editingId = null;
	}

	function saveEdit(preset: SystemPromptPreset) {
		presetsStore.update(preset.id, {
			name: editName,
			description: editDescription,
			content: editContent
		});
		editingId = null;
	}

	async function ensureConversation(): Promise<boolean> {
		if (!conversationsStore.activeConversation) {
			await conversationsStore.createConversation();
		}
		return !!conversationsStore.activeConversation;
	}

	async function applyPreset(preset: SystemPromptPreset | null) {
		if (!(await ensureConversation())) return;
		const conv = conversationsStore.activeConversation;
		if (!conv) return;
		await chatStore.applySystemPromptContent(conv.id, preset ? preset.content : '');
		open = false;
	}

	async function applyDefault() {
		if (!(await ensureConversation())) return;
		const conv = conversationsStore.activeConversation;
		if (!conv) return;
		await chatStore.applySystemPromptContent(conv.id, '');
		open = false;
	}
</script>

<Dialog.Root bind:open onOpenChange={(o) => { if (!o) { searchQuery = ''; showWizard = false; editingId = null; } onOpenChange?.(o); }}>
	<Dialog.Portal>
		<Dialog.Overlay class="z-9999" />
		<Dialog.Content class="z-9999 !max-h-[85dvh] !max-w-2xl overflow-x-hidden overflow-y-auto">
			<Dialog.Header>
				<Dialog.Title>{t('Prompt presets')}</Dialog.Title>
				<Dialog.Description>{t('Pick a prompt preset for this conversation. It applies from the next message.')}</Dialog.Description>
			</Dialog.Header>

			<div class="space-y-3 py-2">
				<SearchInput bind:value={searchQuery} placeholder={t('Search presets...')} />

				{#if !showWizard}
					<ScrollArea class="h-96 pr-3">
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
								{:else}
									<span class="h-4 w-4 shrink-0" aria-hidden="true"></span>
								{/if}
							</button>

							{#each filtered as preset (preset.id)}
								{@const isActive = activePresetId === preset.id}
								<div class="rounded-md transition-colors hover:bg-accent {isActive
									? 'bg-accent text-accent-foreground'
									: ''}">
									<div class="group flex items-center gap-1">
										<button
											type="button"
											class="min-w-0 flex-1 px-3 py-2 text-left text-sm"
											onclick={() => applyPreset(preset)}
										>
											<span class="min-w-0 max-w-80 truncate">{preset.name}</span>
											{#if preset.description}
												<span class="mt-0.5 block min-w-0 max-w-96 truncate text-xs text-muted-foreground"
													>{preset.description}</span
												>
											{/if}
										</button>

										<button
											type="button"
											class="shrink-0 p-2 text-muted-foreground hover:text-foreground"
											title={t('Edit')}
											onclick={() => startEdit(preset)}
										>
											<Pencil class="h-4 w-4" />
										</button>

										<button
											type="button"
											class="shrink-0 p-2 text-muted-foreground hover:text-foreground"
											title={preset.favorite ? t('Remove from favorites') : t('Add to favorites')}
											onclick={() => presetsStore.toggleFavorite(preset.id)}
										>
											<Star class="h-4 w-4 {preset.favorite ? 'fill-amber-400 text-amber-400' : ''}" />
										</button>

										{#if isActive}
											<span class="shrink-0 p-2 text-foreground">
												<Check class="h-4 w-4" />
											</span>
										{:else}
											<span class="shrink-0 p-2" aria-hidden="true">
												<span class="block h-4 w-4"></span>
											</span>
										{/if}

										<button
											type="button"
											class="shrink-0 p-2 text-muted-foreground opacity-0 transition-opacity hover:text-destructive group-hover:opacity-100"
											title={t('Delete')}
											onclick={() => {
												presetsStore.remove(preset.id);
												if (editingId === preset.id) cancelEdit();
											}}
										>
											<Trash2 class="h-4 w-4" />
										</button>
									</div>

									{#if editingId === preset.id}
										<div class="space-y-2 border-t border-border/40 px-3 py-3">
											<Label for="edit-preset-name">{t('Preset name')}</Label>
											<Input id="edit-preset-name" bind:value={editName} />

											<Label for="edit-preset-description">{t('Short description (shown in the picker)')}</Label>
											<Input id="edit-preset-description" bind:value={editDescription} />

											<Label for="edit-preset-content">{t('System prompt draft')}</Label>
											<Textarea id="edit-preset-content" bind:value={editContent} rows={6} class="font-mono text-xs" />

											<div class="flex gap-2">
												<Button type="button" size="sm" onclick={() => saveEdit(preset)}>
													<Check class="mr-1 h-3.5 w-3.5" />
													{t('Save')}
												</Button>
												<Button type="button" variant="ghost" size="sm" onclick={cancelEdit}>
													<X class="mr-1 h-3.5 w-3.5" />
													{t('Cancel')}
												</Button>
											</div>
										</div>
									{/if}
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
