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

	let request = $state('');
	let name = $state('');
	let description = $state('');
	let draft = $state('');
	let generating = $state(false);
	let error = $state('');

	function reset() {
		request = '';
		name = '';
		description = '';
		draft = '';
		error = '';
	}

	/** Parse the model's JSON answer; fall back to treating the whole output as content. */
	function parseGenerated(raw: string): { description: string; content: string } {
		let cleaned = raw.trim().replace(/^```(?:json)?\s*/i, '').replace(/```\s*$/, '');
		const jsonStart = cleaned.indexOf('{');
		const jsonEnd = cleaned.lastIndexOf('}');
		if (jsonStart !== -1 && jsonEnd > jsonStart) {
			cleaned = cleaned.slice(jsonStart, jsonEnd + 1);
		}
		try {
			const obj = JSON.parse(cleaned);
			if (obj && typeof obj.content === 'string') {
				return {
					description: typeof obj.description === 'string' ? obj.description.trim() : '',
					content: obj.content.trim()
				};
			}
		} catch {
			// not JSON — fall through
		}
		return { description: '', content: cleaned };
	}

	async function generate() {
		if (!request.trim() || generating) return;
		generating = true;
		error = '';
		draft = '';
		description = '';
		try {
			const model = config().model as string | undefined;
			await new Promise<void>((resolve) => {
				void ChatService.sendMessage(
					[{ role: 'user', content: `${PRESET_WIZARD_META_PROMPT}\n\n${request.trim()}` }],
					{
						...(model ? { model } : {}),
						temperature: 0.6,
						max_tokens: 500,
						stream: false,
						onComplete: (content: string) => {
							const parsed = parseGenerated(content);
							draft = parsed.content;
							description = parsed.description;
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
			description: description.trim() || request.trim() || undefined,
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
		<Dialog.Content class="z-9999 !max-h-[85dvh] max-w-2xl overflow-y-auto">
			<Dialog.Header>
				<Dialog.Title>{t('Create a prompt preset')}</Dialog.Title>
				<Dialog.Description>{t('Describe the personality or expert role — the model drafts the system prompt for you to review.')}</Dialog.Description>
			</Dialog.Header>

			<div class="space-y-4 py-2">
				<div class="space-y-2">
					<Label for="preset-request">{t('What should the preset do?')}</Label>
					<Textarea
						id="preset-request"
						bind:value={request}
						rows={3}
						placeholder={t('A strict code reviewer who demands citations for every claim…')}
					/>
				</div>

				<Button type="button" onclick={generate} disabled={!request.trim() || generating}>
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

						<Label for="preset-description">{t('Short description (shown in the picker)')}</Label>
						<Input
							id="preset-description"
							bind:value={description}
							placeholder={t('Short description (shown in the picker)')}
						/>

						<Label for="preset-content">{t('System prompt draft')}</Label>
						<Textarea
							id="preset-content"
							bind:value={draft}
							rows={10}
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
