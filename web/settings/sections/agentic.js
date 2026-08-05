/**
 * agentic.js — Agent B: Agentic settings section.
 * Max turns, tool-call toggles, persona-switch tools, title generation.
 */
import { t, updateConfig, config } from '../../kernel/index.js';
import { checkboxField, sliderField, textField, sectionCard } from '../fields.js';

export function renderAgenticSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  root.appendChild(sectionCard(t('Agentic loop')));
  root.lastChild.appendChild(
    sliderField(
      t('Max agentic turns'),
      Number(config().agenticMaxTurns ?? 10),
      1, 100, 1,
      t('How many tool-call rounds the model may run before returning.'),
      (v) => updateConfig({ agenticMaxTurns: v })
    )
  );
  root.lastChild.appendChild(
    checkboxField(
      t('Prompt preset switching tools'),
      config().presetToolsEnabled,
      t('Give the model list_presets and change_preset tools so it can switch the conversation persona (permission is still asked per call).'),
      (v) => updateConfig({ presetToolsEnabled: v })
    )
  );

  root.appendChild(sectionCard(t('Title generation')));
  root.lastChild.appendChild(
    checkboxField(
      t('Use first line as title'),
      config().titleGenerationUseFirstLine,
      t('Title new conversations from the first message line.'),
      (v) => updateConfig({ titleGenerationUseFirstLine: v })
    )
  );
  root.lastChild.appendChild(
    checkboxField(
      t('Generate title with LLM'),
      config().titleGenerationUseLLM,
      t('Ask the model to draft a title after the first exchange.'),
      (v) => updateConfig({ titleGenerationUseLLM: v })
    )
  );
  root.lastChild.appendChild(
    textField(
      t('Title prompt template'),
      config().titleGenerationPrompt ?? '',
      t('Template with {{USER}} / {{ASSISTANT}} placeholders.'),
      (v) => updateConfig({ titleGenerationPrompt: v })
    )
  );

  return root;
}
