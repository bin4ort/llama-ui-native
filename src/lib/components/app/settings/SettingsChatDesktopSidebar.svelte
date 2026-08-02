<script lang="ts">
	import { ICON_CLASS_DEFAULT } from '$lib/constants/css-classes';
	import { Settings as SettingsIcon } from '@lucide/svelte';
	import type { SettingsSection, SettingsSectionTitle } from '$lib/types';
	import { tr } from '$lib/stores/i18n.svelte';

	interface Props {
		sections: SettingsSection[];
		isActive: (section: SettingsSection) => boolean;
		getHref?: (section: SettingsSection) => string;
		onSectionChange?: (section: SettingsSectionTitle) => void;
	}

	let { sections, isActive, getHref, onSectionChange }: Props = $props();

	function tLabel(title: string): string {
		switch(title) {
			case 'Settings': return tr.Settings;
			case 'General': return tr.General;
			case 'Display': return tr.Display;
			case 'Sampling': return tr.Sampling;
			case 'Penalties': return tr.Penalties;
			case 'Tools': return tr.Tools;
			case 'Agentic': return tr.Agentic;
			case 'Developer': return tr.Developer;
			case 'Import / Export': return tr.ImpExp;
			default: return title;
		}
	}
</script>

<div class="sticky top-2 hidden w-64 flex-col self-start bg-background py-4 md:flex gap-6">
	<div class="flex items-center gap-2 py-2">
		<SettingsIcon class="h-5 w-5 md:h-6 md:w-6" />
		<h1 class="text-xl font-semibold md:text-2xl">{tr.Settings}</h1>
	</div>
	<nav class="space-y-1">
		{#each sections as section (section.title)}
			{#if getHref}
				<a class="flex w-full cursor-pointer items-center gap-3 rounded-lg px-3 py-2 text-left text-sm no-underline transition-colors hover:bg-accent {isActive(section) ? 'bg-accent text-accent-foreground' : 'text-muted-foreground'}" href={getHref(section)}>
					<section.icon class={ICON_CLASS_DEFAULT} />
					<span class="ml-2">{tLabel(section.title)}</span>
				</a>
			{:else}
				<button class="flex w-full cursor-pointer items-center gap-3 rounded-lg px-3 py-2 text-left text-sm transition-colors hover:bg-accent {isActive(section) ? 'bg-accent text-accent-foreground' : 'text-muted-foreground'}" onclick={() => onSectionChange?.(section.title)}>
					<section.icon class={ICON_CLASS_DEFAULT} />
					<span class="ml-2">{tLabel(section.title)}</span>
				</button>
			{/if}
		{/each}
	</nav>
	<div class="mt-auto pt-4 text-center opacity-50 text-[11px]">
		<a href="https://github.com" target="_blank" rel="noopener" class="text-muted-foreground no-underline hover:underline" title="Llama UI Native v0.3.0 (build 0x07D1E)">Llama UI Native v0.3.0 (build 0x07D1E)</a>
	</div>
</div>
