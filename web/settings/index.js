/**
 * settings/index.js — Agent B entry: settings + MCP pages.
 * Rendered into the page-root slot via the router (web/index.js).
 */
import { t } from '../kernel/index.js';
import { renderGeneralSection } from './sections/general.js';
import { renderDisplaySection } from './sections/display.js';
import { renderSamplingSection, renderPenaltiesSection } from './sections/sampling.js';
import { renderToolsSection } from './sections/tools.js';
import { renderAgenticSection } from './sections/agentic.js';
import { renderDeveloperSection } from './sections/developer.js';
import { renderImportExportSection } from './sections/import-export.js';
import { renderPresetsPage } from './presets.js';
import { renderMcpServersPage } from './mcp.js';

export const SETTINGS_SECTIONS = [
  ['general', 'General'],
  ['display', 'Display'],
  ['sampling', 'Sampling'],
  ['penalties', 'Penalties'],
  ['tools', 'Tools'],
  ['agentic', 'Agentic'],
  ['developer', 'Developer'],
  ['import-export', 'Import-Export']
];

const RENDERERS = {
  general: renderGeneralSection,
  display: renderDisplaySection,
  sampling: renderSamplingSection,
  penalties: renderPenaltiesSection,
  tools: renderToolsSection,
  agentic: renderAgenticSection,
  developer: renderDeveloperSection,
  'import-export': renderImportExportSection
};

export function renderSettingsPage(container, params) {
  const section = (params && params[0]) || 'general';
  const valid = SETTINGS_SECTIONS.some(([slug]) => slug === section);

  const root = document.createElement('div');
  root.className = 'flex h-full min-h-0';

  // Section nav
  const nav = document.createElement('aside');
  nav.className = 'w-56 shrink-0 border-r border-border/40 p-3 overflow-y-auto';
  for (const [slug, label] of SETTINGS_SECTIONS) {
    const a = document.createElement('a');
    a.href = `#/settings/${slug}`;
    a.className =
      'block rounded-md px-3 py-2 text-sm hover:bg-accent ' +
      (slug === section ? 'bg-accent font-medium' : 'text-muted-foreground');
    a.textContent = t(label);
    nav.appendChild(a);
  }
  root.appendChild(nav);

  // Content
  const content = document.createElement('div');
  content.className = 'min-w-0 flex-1 overflow-y-auto p-6';
  content.id = 'settings-content';
  root.appendChild(content);

  if (!valid) {
    const p = document.createElement('p');
    p.className = 'text-sm text-muted-foreground';
    p.textContent = `${t('Unknown settings section')}: ${section}`;
    content.appendChild(p);
  } else {
    const render = RENDERERS[section];
    content.appendChild(render());
  }

  // Presets live in the tools section as a collapsible group in the current
  // app; here they get their own sub-page reachable from Tools.
  if (section === 'tools') {
    const link = document.createElement('a');
    link.href = '#/settings/presets';
    link.className = 'mt-4 inline-block text-sm text-primary underline';
    link.textContent = t('Manage prompt presets…');
    content.appendChild(link);
  }

  container.replaceChildren(root);
}

/** #/settings/presets — preset manager page (Agent B). */
export function renderPresetsRoute(container) {
  renderPresetsPage(container);
}

/** #/mcp-servers — MCP servers page (Agent B). */
export function renderMcpRoute(container) {
  renderMcpServersPage(container);
}
