/**
 * PresetStore — system prompt preset (persona) library.
 *
 * The library is persisted as a JSON string inside the settings config
 * (`systemPromptPresets`), so it rides along with the existing localStorage
 * persistence and the settings export/import for free.
 */
import { config, settingsStore } from '$lib/stores/settings.svelte';
import { SETTINGS_KEYS } from '$lib/constants';
import type { SystemPromptPreset } from '$lib/types';

function parsePresets(raw: string | undefined): SystemPromptPreset[] {
	if (!raw) return [];
	try {
		const parsed = JSON.parse(raw);
		if (!Array.isArray(parsed)) return [];
		return parsed.filter(
			(p): p is SystemPromptPreset =>
				!!p && typeof p === 'object' && typeof p.name === 'string' && typeof p.content === 'string'
		);
	} catch {
		return [];
	}
}

class PresetStore {
	get presets(): SystemPromptPreset[] {
		return parsePresets(config()[SETTINGS_KEYS.SYSTEM_PROMPT_PRESETS] as string | undefined);
	}

	get favorites(): SystemPromptPreset[] {
		return this.presets.filter((p) => p.favorite);
	}

	getById(id: string): SystemPromptPreset | undefined {
		return this.presets.find((p) => p.id === id);
	}

	/** Find a preset by exact name (used by the change_preset tool). */
	getByName(name: string): SystemPromptPreset | undefined {
		return this.presets.find((p) => p.name === name);
	}

	private persist(list: SystemPromptPreset[]): void {
		settingsStore.updateConfig(SETTINGS_KEYS.SYSTEM_PROMPT_PRESETS, JSON.stringify(list));
	}

	add(preset: Omit<SystemPromptPreset, 'id'>): SystemPromptPreset {
		const entry: SystemPromptPreset = {
			id: crypto.randomUUID(),
			name: preset.name.trim(),
			description: preset.description?.trim() || undefined,
			content: preset.content,
			favorite: false
		};
		this.persist([...this.presets, entry]);
		return entry;
	}

	update(id: string, patch: Partial<Omit<SystemPromptPreset, 'id'>>): void {
		this.persist(
			this.presets.map((p) =>
				p.id === id
					? {
							...p,
							name: patch.name !== undefined ? patch.name.trim() : p.name,
							description:
								patch.description !== undefined
									? patch.description.trim() || undefined
									: p.description,
							content: patch.content !== undefined ? patch.content : p.content,
							favorite: patch.favorite !== undefined ? patch.favorite : p.favorite
						}
					: p
			)
		);
	}

	remove(id: string): void {
		this.persist(this.presets.filter((p) => p.id !== id));
	}

	toggleFavorite(id: string): void {
		const preset = this.getById(id);
		if (preset) this.update(id, { favorite: !preset.favorite });
	}

	/**
	 * Resolve the effective system prompt for a preset selection.
	 * @param presetId the selected preset id, or null/PRESET_DEFAULT_ID for the
	 *                 default system prompt from settings.
	 */
	resolveContent(presetId: string | null): string {
		if (!presetId) return config()[SETTINGS_KEYS.SYSTEM_MESSAGE]?.toString().trim() ?? '';
		const preset = this.getById(presetId);
		return preset ? preset.content.trim() : '';
	}
}

export const presetsStore = new PresetStore();

export const presetList = () => presetsStore.presets;
export const presetFavorites = () => presetsStore.favorites;
