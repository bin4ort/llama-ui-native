/**
 * display.js — Agent B: Display settings section. Registry-driven (12
 * checkboxes, old order).
 */
import { t, updateConfig, config } from '../../kernel/index.js';
import { registrySection } from '../fields.js';
import { FIELDS } from '../registry.js';

export function renderDisplaySection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';
  root.appendChild(
    registrySection(FIELDS.display, { config, updateConfig, translate: (s) => t(s) })
  );
  return root;
}
