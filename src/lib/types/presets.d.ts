/**
 * System prompt preset (persona) — a reusable system prompt the user can
 * create via the wizard, edit in settings, star as favorite and apply to a
 * conversation (or let the model switch via the change_preset tool).
 */

export interface SystemPromptPreset {
	id: string;
	name: string;
	/** Short one-line description shown in the picker. */
	description?: string;
	/** The system prompt content. */
	content: string;
	/** Starred presets surface in the chat-bar quick picker (max PRESETS_FAVORITES_MAX). */
	favorite?: boolean;
}

