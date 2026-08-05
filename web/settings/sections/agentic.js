/**
 * agentic.js — Agent B: Agentic settings section. Registry-driven
 * (agentic turns + MCP timeout, old order).
 */
import { t, updateConfig, config } from '../../kernel/index.js';
import { registrySection } from '../fields.js';
import { FIELDS } from '../registry.js';

export function renderAgenticSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';
  root.appendChild(
    registrySection(FIELDS.agentic, { config, updateConfig, translate: (s) => t(s) })
  );
  return root;
}
