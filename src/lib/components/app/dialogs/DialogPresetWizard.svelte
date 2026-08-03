<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { config } from '$lib/stores/settings.svelte';
	import { ChatService } from '$lib/services';
	import { presetsStore } from '$lib/stores/presets.svelte';
	import { PRESET_WIZARD_META_PROMPT } from '$lib/constants/presets';
	import * as Dialog from '$lib/components/ui/dialog';
	import { Button } from '$lib/components/ui/button';
	import { Textarea } from '$lib/components/ui/textarea';
	import { Input } from '$lib/components/ui/input';
	import { Label } from '$lib/components/ui/label';
	import { Loader2, Sparkles, Save } from '@lucide/svelte';

	interface Props {
		open?: boolean;
		onOpenChange?: (open: boolean) => void;
		onSaved?: (id: string) => void;
	}

	let { open = $bindable(), onOpenChange, onSaved }: Props = $props();

	let description = $state('');
	let name = $state('');
	let draft = $state('');
	let generating = $state(false);
	let error = $state('');

	function reset() {
		description = '';
		name = '';
		draft = '';
		error = '';
	}

	async function generate() {
		if (!description.trim() || generating) return;
		generating = true;
		error = '';
		draft = '';
		try {
			const model = config().model as string | undefined;
			await new Promise<void>((resolve) => {
				void ChatService.sendMessage(
					[{ role: 'user', content: `${PRESET_WIZARD_META_PROMPT}\n\n${description.trim()}` }],
					{
						...(model ? { model } : {}),
						temperature: 0.6,
						max_tokens: 400,
						stream: false,
						onChunk: (chunk: string) => {
							draft = chunk;
						},
						onComplete: (content: string) => {
							draft = content;
							resolve();
						},
						onError: (e: Error) => {
							error = e.message;
							resolve();
						}
					}
				);
			});
		} finally {
			generating = false;
		}
	}

	function save() {
		if (!draft.trim() || !name.trim()) return;
		const preset = presetsStore.add({
			name: name.trim(),
			description: description.trim() || undefined,
			content: draft.trim()
		});
		onSaved?.(preset.id);
		reset();
		open = false;
	}
</script>

<Dialog.Root bind:open onOpenChange={(o) => { if (!o) reset(); onOpenChange?.(o); }}>
	<Dialog.Portal>
		<Dialog.Overlay class="z-9999" />
		<Dialog.Content class="z-9999 max-w-xl">
			<Dialog.Header>
				<Dialog.Title>{t('Create a prompt preset')}</Dialog.Title>
				<Dialog.Description>{t('Describe the personality or expert role — the model drafts the system prompt for you to review.')}</Dialog.Description>
			</Dialog.Header>

			<div class="space-y-4 py-2">
				<div class="space-y-2">
					<Label for="preset-description">{t('Preset description')}</Label>
					<Textarea
						id="preset-description"
						bind:value={description}
						rows={3}
						placeholder={t('A strict code reviewer who demands citations for every claim…')}
					/>
				</div>

				<Button type="button" onclick={generate} disabled={!description.trim() || generating}>
					{#if generating}
						<Loader2 class="mr-2 h-4 w-4 animate-spin" />
					{:else}
						<Sparkles class="mr-2 h-4 w-4" />
					{/if}
					{t('Generate')}
				</Button>

				{#if error}
					<p class="text-sm text-destructive">{error}</p>
				{/if}

				{#if draft}
					<div class="space-y-2">
						<Label for="preset-name">{t('Preset name')}</Label>
						<Input id="preset-name" bind:value={name} placeholder={t('Expert Code Reviewer')} />

						<Label for="preset-content">{t('System prompt draft')}</Label>
						<Textarea
							id="preset-content"
							bind:value={draft}
							rows={8}
							class="font-mono text-xs"
						/>
					</div>
				{/if}
			</div>

			<Dialog.Footer>
				<Button variant="outline" onclick={() => (open = false)}>{t('Cancel')}</Button>
				<Button onclick={save} disabled={!draft.trim() || !name.trim()}>
					<Save class="mr-2 h-4 w-4" />
					{t('Save preset')}
				</Button>
			</Dialog.Footer>
		</Dialog.Content>
	</Dialog.Portal>
</Dialog.Root>
