/**
 * sampling.js / penalties.js — Agent B: registry-driven sampling parameter
 * sections (visual parity port: numeric inputs with labels + help + server
 * default placeholders, matching the old UI instead of sliders).
 */
import { t, config, updateConfig, api, log } from '../../kernel/index.js';
import { registrySection, sectionCard } from '../fields.js';
import { FIELDS } from '../registry.js';

let propsCache = {};

export async function loadProps() {
  try {
    const props = await api.getProps();
    propsCache = props?.default_generation_settings?.params ?? {};
  } catch (err) {
    log.warn('LLMUI-API-004', 'props: fetch failed — defaults placeholders unavailable', err?.code ?? '');
  }
  return propsCache;
}

function renderSection(sectionKey, title) {
  const root = document.createElement('div');
  root.className = 'space-y-4';
  const card = sectionCard(t(title));
  const fields = registrySection(FIELDS[sectionKey], {
    config,
    updateConfig,
    props: propsCache,
    translate: (s) => t(s)
  });
  card.appendChild(fields);
  root.appendChild(card);
  return root;
}

export function renderSamplingSection() {
  const el = renderSection('sampling', 'Sampling');
  // The /props defaults arrive async — re-render so the "Default: …"
  // placeholders appear (and reset buttons reflect server drift).
  if (!Object.keys(propsCache).length) {
    loadProps().then(() => {
      const target = document.getElementById('settings-content');
      if (!target) return;
      target.replaceChildren(renderSection('sampling', 'Sampling'));
    });
  }
  return el;
}

export function renderPenaltiesSection() {
  return renderSection('penalties', 'Penalties');
}
