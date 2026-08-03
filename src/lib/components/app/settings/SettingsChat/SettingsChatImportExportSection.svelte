<script lang="ts">
	import { ICON_CLASS_DEFAULT } from '$lib/constants/css-classes';
	import type { Component } from 'svelte';
	import { Button, type ButtonVariant } from '$lib/components/ui/button';
	import { t } from '$lib/stores/i18n.svelte';

	let {
		title,
		description,
		IconComponent,
		buttonText,
		onclick,
		titleClass,
		buttonVariant,
		buttonClass,
		wrapperClass,
		summary
	}: {
		title: string;
		description: string;
		IconComponent: Component;
		buttonText: string;
		onclick: () => void;
		titleClass?: string;
		buttonVariant?: ButtonVariant;
		buttonClass?: string;
		wrapperClass?: string;
		summary?: { show: boolean; verb: string; items: DatabaseConversation[] };
	} = $props();

	let sectionButtonClass = $derived(buttonClass ?? 'justify-start justify-self-start md:w-auto');
	let sectionButtonVariant = $derived(buttonVariant ?? 'outline');
</script>

<div class="grid gap-1 {wrapperClass ?? ''}">
	<h4 class="mt-0 mb-2 text-sm font-medium {titleClass ?? ''}">{t(title)}</h4>

	<p class="mb-4 text-sm text-muted-foreground">{t(description)}</p>

	<Button class={sectionButtonClass} {onclick} variant={sectionButtonVariant}>
		<IconComponent class="mr-2 {ICON_CLASS_DEFAULT}" />

		{t(buttonText)}
	</Button>

	{#if summary && summary.show && summary.items.length > 0}
		<div class="mt-4 grid overflow-x-auto rounded-lg border border-border/50 bg-muted/30 p-4">
			<h5 class="mb-2 text-sm font-medium">
				{t(summary.verb)}
				{summary.items.length} {t(summary.items.length === 1 ? 'conversation' : 'conversations')}
			</h5>

			<ul class="space-y-1 text-sm text-muted-foreground">
				{#each summary.items.slice(0, 10) as conv (conv.id)}
					<li class="truncate">• {conv.name || t('Untitled conversation')}</li>
				{/each}

				{#if summary.items.length > 10}
					<li class="italic">
						{t('and {n} more').replace('{n}', String(summary.items.length - 10))}
					</li>
				{/if}
			</ul>
		</div>
	{/if}
</div>
