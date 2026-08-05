/**
 * settings/index.js — Agent B entry: settings + MCP pages.
 * Visual parity port of the old settings page shell: w-64 sticky sidebar
 * (header + section nav + version footer), max-w-3xl content with the
 * section title row (icon + h3, border-b) and the sticky bottom action bar
 * (Reset to default / Save settings).
 */
import { t, log, settings } from '../kernel/index.js';
import { icon } from './icons.js';
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
  ['general', 'General', 'sliders'],
  ['display', 'Display', 'monitor'],
  ['sampling', 'Sampling', 'funnel'],
  ['penalties', 'Penalties', 'list-restart'],
  ['tools', 'Tools', 'pencil-ruler'],
  ['agentic', 'Agentic', 'bot'],
  ['developer', 'Developer', 'code'],
  ['import-export', 'Import-Export', 'download']
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
  const current = SETTINGS_SECTIONS.find(([slug]) => slug === section) ?? SETTINGS_SECTIONS[0];

  const root = document.createElement('div');
  root.className = 'mx-auto flex h-full w-full flex-col md:pl-8';

  const body = document.createElement('div');
  body.className = 'flex flex-1 flex-col gap-4 md:flex-row';

  // ---- desktop sidebar (w-64 sticky, ported 1:1) ----
  const side = document.createElement('aside');
  side.className = 'sticky top-2 hidden w-64 flex-col self-start bg-background py-4 md:flex gap-6';
  const head = document.createElement('div');
  head.className = 'flex items-center gap-2 py-2';
  head.appendChild(icon('settings', 'h-5 w-5 md:h-6 md:w-6'));
  const h1 = document.createElement('h1');
  h1.className = 'text-xl font-semibold md:text-2xl';
  h1.textContent = t('Settings');
  head.appendChild(h1);
  side.appendChild(head);

  const nav = document.createElement('nav');
  nav.className = 'space-y-1';
  for (const [slug, label, ic] of SETTINGS_SECTIONS) {
    const a = document.createElement('a');
    a.href = `#/settings/${slug}`;
    a.className =
      'flex w-full cursor-pointer items-center gap-3 rounded-lg px-3 py-2 text-left text-sm no-underline transition-colors hover:bg-accent ' +
      (slug === section ? 'bg-accent text-accent-foreground' : 'text-muted-foreground');
    a.appendChild(icon(ic, 'h-4 w-4 shrink-0'));
    const span = document.createElement('span');
    span.className = 'ml-2';
    span.textContent = t(label);
    a.appendChild(span);
    nav.appendChild(a);
  }
  side.appendChild(nav);

  const footerText = `Llama UI Native v${globalThis.APP_VERSION} (build ${globalThis.APP_BUILD})`;
  const sideFooter = document.createElement('div');
  sideFooter.className = 'mt-auto pt-4 text-center opacity-50 text-[11px]';
  const ver = document.createElement('span');
  ver.className = 'text-muted-foreground';
  ver.textContent = footerText;
  sideFooter.appendChild(ver);
  side.appendChild(sideFooter);
  body.appendChild(side);

  // ---- content (max-w-3xl) ----
  const content = document.createElement('div');
  content.className = 'mx-auto max-w-3xl flex-1';
  content.id = 'settings-content';
  const inner = document.createElement('div');
  inner.className = 'space-y-6 p-4 md:p-6 md:pt-28';

  const titleRow = document.createElement('div');
  titleRow.className = 'mb-6 flex items-center gap-2 border-b border-border/30 pb-6 md:flex';
  titleRow.appendChild(icon(current[2], 'h-5 w-5'));
  const h3 = document.createElement('h3');
  h3.className = 'text-lg font-semibold';
  h3.textContent = t(current[1]);
  titleRow.appendChild(h3);
  inner.appendChild(titleRow);

  const groups = document.createElement('div');
  groups.className = 'space-y-6';
  if (!valid) {
    const p = document.createElement('p');
    p.className = 'text-sm text-muted-foreground';
    p.textContent = `${t('Unknown settings section')}: ${section}`;
    groups.appendChild(p);
  } else {
    groups.appendChild(RENDERERS[section]());
  }
  if (section === 'tools') {
    const link = document.createElement('a');
    link.href = '#/settings/presets';
    link.className = 'mt-4 inline-block text-sm text-primary underline';
    link.textContent = t('Manage prompt presets…');
    groups.appendChild(link);
  }
  inner.appendChild(groups);

  const note = document.createElement('div');
  note.className = 'mt-8 border-t border-border/30 pt-6';
  const p = document.createElement('p');
  p.className = 'text-xs text-muted-foreground';
  p.textContent = t("Settings are saved in browser's localStorage");
  note.appendChild(p);
  inner.appendChild(note);
  content.appendChild(inner);
  body.appendChild(content);
  root.appendChild(body);

  // ---- sticky bottom action bar ----
  const footer = document.createElement('div');
  footer.className = 'sticky bottom-0 mx-auto mt-4 flex w-full justify-between p-6';
  const left = document.createElement('div');
  left.className = 'flex gap-2';
  const resetBtn = document.createElement('button');
  resetBtn.type = 'button';
  resetBtn.className = 'h-9 inline-flex items-center gap-2 rounded-md border border-input px-4 text-sm hover:bg-accent';
  resetBtn.appendChild(icon('refresh-ccw', 'h-3 w-3'));
  resetBtn.appendChild(document.createTextNode(t('Reset to default')));
  resetBtn.addEventListener('click', () => {
    if (confirm(t('Reset all settings to defaults? This cannot be undone.'))) {
      settings.resetConfig();
      location.reload();
    }
  });
  left.appendChild(resetBtn);
  footer.appendChild(left);

  const saveBtn = document.createElement('button');
  saveBtn.type = 'button';
  saveBtn.className = 'h-9 rounded-md bg-primary px-4 text-sm font-medium text-primary-foreground hover:bg-primary/90';
  saveBtn.textContent = t('Save settings');
  saveBtn.addEventListener('click', () => {
    settings.saveConfig();
    log.info('LLMUI-CFG-010', 'settings saved');
  });
  footer.appendChild(saveBtn);
  root.appendChild(footer);

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
