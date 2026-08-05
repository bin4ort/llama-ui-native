/**
 * display.js — Agent B: Display settings section.
 * Theme-independent rendering options + custom CSS/JSON.
 */
import { t, updateConfig, config } from '../../kernel/index.js';
import { checkboxField, textField, sectionCard } from '../fields.js';

const TOGGLES = [
  ['showSystemMessage', 'Show system message', 'Render the persona/context row as a message bubble.'],
  ['showRawModelNames', 'Show raw model names', 'Display full model ids instead of short labels.'],
  ['showModelQuantization', 'Show quantization', 'Display Q4_K_M-style quant labels next to models.'],
  ['showModelTags', 'Show model tags', 'Display model tags/context size labels.'],
  ['fullHeightCodeBlocks', 'Full-height code blocks', 'Do not cap code block height.'],
  ['disableAutoScroll', 'Disable auto-scroll', 'Do not follow new tokens when scrolled up.'],
  ['alwaysShowSidebarOnDesktop', 'Always show sidebar on desktop', 'Keep the conversation list expanded.'],
  ['renderUserContentAsMarkdown', 'Render user messages as markdown', 'Format user text with the markdown pipeline.'],
  ['renderThinkingAsMarkdown', 'Render thinking as markdown', 'Format reasoning blocks with markdown.'],
  ['showMessageStats', 'Show message stats', 'Display token/timing stats under messages.'],
  ['showAgenticTurnStats', 'Show agentic turn stats', 'Display per-turn statistics in agentic loops.'],
  ['showThoughtInProgress', 'Show thought in progress', 'Render an animated thinking indicator while reasoning.'],
  ['alwaysShowToolCallContent', 'Always show tool call content', 'Expand tool call payloads by default.']
];

export function renderDisplaySection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  const card = sectionCard(t('Display'));
  root.appendChild(card);
  for (const [key, label, help] of TOGGLES) {
    card.appendChild(
      checkboxField(t(label), Boolean(config()[key]), t(help), (v) => updateConfig({ [key]: v }))
    );
  }

  root.appendChild(sectionCard(t('Custom styles')));
  const css = document.createElement('textarea');
  css.className = 'mt-1 w-full rounded-md border border-input bg-background p-2 font-mono text-xs';
  css.rows = 6;
  css.value = config().customCss ?? '';
  css.addEventListener('input', () => updateConfig({ customCss: css.value }));
  root.lastChild.appendChild(css);

  root.appendChild(sectionCard(t('Custom JSON')));
  const json = document.createElement('textarea');
  json.className = 'mt-1 w-full rounded-md border border-input bg-background p-2 font-mono text-xs';
  json.rows = 6;
  json.value = config().customJson ?? '';
  json.addEventListener('input', () => updateConfig({ customJson: json.value }));
  root.lastChild.appendChild(json);

  return root;
}
