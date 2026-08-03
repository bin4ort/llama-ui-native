/**
 * System prompt preset (persona) constants: config keys, chat-bar picker
 * limits and the model-callable personality tools.
 */

import type { OpenAIToolDefinition } from '$lib/types';

/** Config key storing the preset library as a JSON string (like mcpServers). */
export const SYSTEM_PROMPT_PRESETS_KEY = 'systemPromptPresets';

/** Config key gating the model-callable personality tools. */
export const PRESET_TOOLS_ENABLED_KEY = 'presetToolsEnabled';

/** Sentinel used to signal "the default system prompt" in pickers. */
export const PRESET_DEFAULT_ID = '__default__';

/** Maximum number of favorites shown in the chat-bar quick picker. */
export const PRESETS_FAVORITES_MAX = 5;

/** Model-callable tools for switching personas mid-conversation. */
export const PRESET_LIST_TOOL = 'list_presets';
export const PRESET_CHANGE_TOOL = 'change_preset';

/**
 * Meta-prompt used by the preset wizard: the loaded model drafts a system
 * prompt from the user's plain-language description. Result is always shown
 * to the user for review — never auto-saved.
 */
export const PRESET_WIZARD_META_PROMPT =
	'You are a system prompt designer. Turn the user\'s description into a concise, ' +
	'effective system prompt that defines a personality, role and expert knowledge. ' +
	'Write 2–5 sentences, second person, imperative mood. Keep it focused: no formatting ' +
	'instructions, no repetition of the request.';

/** List all available presets (names + descriptions) so the model can pick. */
export function buildListPresetsToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: PRESET_LIST_TOOL,
			description:
				'List the available personality presets (names and descriptions). Use this before change_preset to see what can be selected.',
			parameters: { type: 'object', properties: {}, required: [] }
		}
	};
}

/** Switch the active conversation to a personality preset by name. */
export function buildChangePresetToolDefinition(): OpenAIToolDefinition {
	return {
		type: 'function',
		function: {
			name: PRESET_CHANGE_TOOL,
			description:
				'Switch the current conversation to a personality preset by its exact name (see list_presets). The new persona applies to the next message. Ask the user before switching unless they explicitly asked for it.',
			parameters: {
				type: 'object',
				properties: {
					name: {
						type: 'string',
						description: 'Exact name of the preset to switch to.'
					}
				},
				required: ['name']
			}
		}
	};
}
