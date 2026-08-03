<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { presetsStore } from '$lib/stores/presets.svelte';
	import { conversationsStore } from '$lib/stores/conversations.svelte';
	import { chatStore } from '$lib/stores/chat.svelte';
	import { PRESETS_FAVORITES_MAX } from '$lib/constants/presets';
	import type { SystemPromptPreset } from '$lib/types';
	import * as DropdownMenu from '$lib/components/ui/dropdown-menu';
	import { buttonVariants } from '$lib/components/ui/button';
	import { cn } from '$lib/components/ui/utils';
	import { BadgeCheck, Sparkles, Check, ChevronDown } from '@lucide/svelte';
	import DialogPresetPicker from '$lib/components/app/dialogs/DialogPresetPicker.svelte';

	interface Props {
		disabled?: boolean;
	}

	let { disabled = false }: Props = $props();

	let pickerOpen = $state(false);

	const activePreset = $derived.by(() => {
		const am = conversationsStore.activeMessages;
		const sys = am.find((m) => m.role === 'system');
		const content = typeof sys?.content === 'string' ? sys.content.trim() : '';
		if (!content) return null;
		return presetsStore.presets.find((p) => p.content.trim() === content) ?? null;
	});

	const favorites = $derived(presetsStore.favorites.slice(0, PRESETS_FAVORITES_MAX));

	function apply(preset: SystemPromptPreset | null) {
		const conv = conversationsStore.activeConversation;
		if (!conv) return;
		void chatStore.applySystemPromptContent(conv.id, preset ? preset.content : '');
	}

	function applyDefault() {
		const conv = conversationsStore.activeConversation;
		if (!conv) return;
		void chatStore.applySystemPromptContent(conv.id, '');
	}
</script>

<DropdownMenu.Root>
	<DropdownMenu.Trigger
		{disabled}
		class={cn(
			buttonVariants({ variant: 'ghost', size: 'sm' }),
			'h-8 cursor-pointer gap-1 px-2 text-xs'
		)}
		title={t('Choose a prompt preset')}
	>
		{#if activePreset}
			<BadgeCheck class="h-3.5 w-3.5 shrink-0 text-amber-400" />
		{:else}
			<Sparkles class="h-3.5 w-3.5 shrink-0 text-muted-foreground" />
		{/if}
		<span class="max-w-24 truncate">{activePreset?.name ?? t('Default')}</span>
		<ChevronDown class="h-3 w-3 shrink-0 text-muted-foreground" />
	</DropdownMenu.Trigger>

	<DropdownMenu.Content align="start" class="w-64">
		<DropdownMenu.Label>{t('Prompt presets')}</DropdownMenu.Label>

		<DropdownMenu.Item
			class="flex cursor-pointer items-center justify-between gap-2"
			onclick={applyDefault}
		>
			<span>{t('Default')}</span>
			{#if !activePreset}
				<Check class="h-4 w-4" />
			{/if}
		</DropdownMenu.Item>

		<DropdownMenu.Separator />

		{#each favorites as preset (preset.id)}
			<DropdownMenu.Item
				class="flex cursor-pointer items-center justify-between gap-2"
				onclick={() => apply(preset)}
			>
				<span class="truncate">{preset.name}</span>
				{#if activePreset?.id === preset.id}
					<Check class="h-4 w-4 shrink-0" />
				{/if}
			</DropdownMenu.Item>
		{/each}

		<DropdownMenu.Separator />

		<DropdownMenu.Item class="cursor-pointer" onclick={() => (pickerOpen = true)}>
			{t('All presets…')}
		</DropdownMenu.Item>
	</DropdownMenu.Content>
</DropdownMenu.Root>

<DialogPresetPicker bind:open={pickerOpen} />
