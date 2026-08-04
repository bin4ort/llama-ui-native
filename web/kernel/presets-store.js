/**
 * presets-store.js — preset library (in LlamaUi.config.systemPromptPresets,
 * JSON string) + one-time built-in seeding (marker: systemPromptPresetsSeeded).
 */
import { configStore, saveConfig } from './settings-store.js';
import { log } from './logger.js';

const SEEDED_KEY = 'systemPromptPresetsSeeded';
export const PRESET_NAME_MAX_LENGTH = 50;

export const BUILTIN_PRESETS = [
  {
    name: 'Expert Programmer',
    description: 'Senior software engineer, clean idiomatic code',
    content:
      'You are a senior software engineer. Write clean, idiomatic, well-structured code. Explain trade-offs briefly and point out edge cases and potential bugs.'
  },
  {
    name: 'Strict Code Reviewer',
    description: 'Rigorous reviewer demanding evidence for every claim',
    content:
      'You are a strict code reviewer. For every claim or recommendation cite the specific lines or docs you base it on. Prefer correctness and maintainability over speed, and always flag security issues.'
  },
  {
    name: 'Creative Writer',
    description: 'Engaging, vivid prose and storytelling',
    content:
      'You are a creative writer. Use vivid, precise language, vary sentence rhythm, and build scenes with strong imagery. Match the tone the user asks for.'
  },
  {
    name: 'Translator',
    description: 'Faithful, natural translation between languages',
    content:
      'You are a professional translator. Translate faithfully while keeping the tone and register natural in the target language. Preserve formatting and note ambiguous passages.'
  },
  {
    name: 'Data Analyst',
    description: 'Thorough analysis of tables, logs and exports',
    content:
      'You are a data analyst. When given tables or data, summarize patterns, compute relevant statistics, and point out outliers or quality issues. Cite the numbers you base conclusions on.'
  }
];

export function getPresets() {
  try {
    const raw = configStore.get().systemPromptPresets;
    const parsed = JSON.parse(raw || '[]');
    return Array.isArray(parsed) ? parsed : [];
  } catch (err) {
    log.error('LLMUI-PRS-000', 'presets: library parse failed', String(err));
    return [];
  }
}

export function persistPresets(list) {
  updateConfig({ systemPromptPresets: JSON.stringify(list) });
}

export function addPreset({ name, description, content }) {
  const presets = getPresets();
  const entry = {
    id: crypto.randomUUID(),
    name: name.trim().slice(0, PRESET_NAME_MAX_LENGTH),
    description: description?.trim() || undefined,
    content,
    favorite: false
  };
  persistPresets([...presets, entry]);
  return entry;
}

export function updatePreset(id, patch) {
  persistPresets(
    getPresets().map((p) =>
      p.id === id
        ? {
            ...p,
            name: patch.name !== undefined ? patch.name.trim().slice(0, PRESET_NAME_MAX_LENGTH) : p.name,
            description: patch.description !== undefined ? patch.description.trim() || undefined : p.description,
            content: patch.content !== undefined ? patch.content : p.content,
            favorite: patch.favorite !== undefined ? patch.favorite : p.favorite
          }
        : p
    )
  );
}

export function removePreset(id) {
  persistPresets(getPresets().filter((p) => p.id !== id));
}

export function toggleFavorite(id) {
  const preset = getPresets().find((p) => p.id === id);
  if (preset) updatePreset(id, { favorite: !preset.favorite });
}

/** One-time seeding of the built-in presets (marker key kept for compatibility). */
export function seedBuiltinPresets() {
  if (localStorage.getItem(SEEDED_KEY) !== null) return;
  if (getPresets().length > 0) {
    localStorage.setItem(SEEDED_KEY, '1');
    return;
  }
  try {
    persistPresets(BUILTIN_PRESETS.map((p) => ({ ...p, id: crypto.randomUUID(), favorite: false })));
    localStorage.setItem(SEEDED_KEY, '1');
  } catch (err) {
    log.error('LLMUI-PRS-004', 'presets: seeding marker write failed', String(err));
  }
}

import { updateConfig } from './settings-store.js';
