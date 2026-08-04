/**
 * SettingsToolsStore — staged "always allow" tool permissions.
 *
 * Changes made in the Tools settings tab are staged here and only applied
 * when "Save settings" is confirmed. If staged changes include NEW always-allow
 * grants, Save asks for verification first (see VerificationDialog); when the
 * verification is cancelled the staged grants are discarded while the rest of
 * the settings still save.
 */
import { permissionsStore } from '$lib/stores/permissions.svelte';
import { SvelteSet } from 'svelte/reactivity';

/** Full desired always-allow set while staging is active; null = no staged changes. */
let pending = $state<SvelteSet<string> | null>(null);

class SettingsToolsStore {
	/** True when the user has staged changes that are not yet applied. */
	get hasPending(): boolean {
		return pending !== null;
	}

	/** Checkbox state: staged value if present, otherwise the applied value. */
	isChecked(key: string): boolean {
		return pending ? pending.has(key) : permissionsStore.hasTool(key);
	}

	/** Toggle a staged change; initializes the staging set from the applied state. */
	toggle(key: string): void {
		if (pending === null) {
			pending = new SvelteSet<string>([...permissionsStore.tools]);
		}
		if (pending.has(key)) pending.delete(key);
		else pending.add(key);
	}

	/** Keys staged to be granted that are not yet allowed. */
	get additions(): string[] {
		const staged = pending;
		if (!staged) return [];
		return [...staged].filter((key) => !permissionsStore.hasTool(key));
	}

	/** Keys staged to be revoked that are currently allowed. */
	get removals(): string[] {
		const staged = pending;
		if (!staged) return [];
		return [...permissionsStore.tools].filter((key) => !staged.has(key));
	}

	/** Apply the staged set to the permissions store (called after verification). */
	apply(): void {
		if (!pending) return;
		const additions = this.additions;
		const removals = this.removals;
		if (additions.length > 0) permissionsStore.allowTools(additions);
		for (const key of removals) permissionsStore.revokeTool(key);
		pending = null;
	}

	/** Drop the staged changes, reverting checkboxes to the applied state. */
	discard(): void {
		pending = null;
	}
}

export const settingsToolsStore = new SettingsToolsStore();
