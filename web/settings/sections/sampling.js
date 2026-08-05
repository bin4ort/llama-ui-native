/**
 * sampling.js / penalties.js — Agent B: sampling parameter sliders.
 * Config keys and defaults match the current app (server /props).
 */
import { t, updateConfig, config } from '../../kernel/index.js';
import { sliderField, sectionCard } from '../fields.js';

const D = {
  temperature: 0.8, top_k: 40, top_p: 0.95, min_p: 0.05, typ_p: 1.0,
  xtc_probability: 0.0, xtc_threshold: 0.1,
  dynatemp_range: 0.0, dynatemp_exponent: 1.0
};

const P = {
  repeat_last_n: 64, repeat_penalty: 1.0, presence_penalty: 0.0,
  frequency_penalty: 0.0, dry_multiplier: 0.0, dry_base: 1.75,
  dry_allowed_length: 2, dry_penalty_last_n: -1
};

function read(key) {
  const v = config()[key];
  return v === undefined || v === null || v === '' ? D[key] ?? P[key] : Number(v);
}

function bind(card, label, key, min, max, step, help, format) {
  card.appendChild(
    sliderField(label, read(key), min, max, step, help, (v) => updateConfig({ [key]: v }), format)
  );
}

export function renderSamplingSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';
  const card = sectionCard(t('Sampling parameters'));
  root.appendChild(card);
  bind(card, 'Temperature', 'temperature', 0, 2, 0.01, t('Higher = more creative, lower = more deterministic.'));
  bind(card, 'Top K', 'top_k', 0, 100, 1);
  bind(card, 'Top P', 'top_p', 0, 1, 0.01, t('Nucleus sampling threshold.'));
  bind(card, 'Min P', 'min_p', 0, 1, 0.01);
  bind(card, 'Typical P', 'typ_p', 0, 2, 0.01);
  bind(card, 'XTC probability', 'xtc_probability', 0, 1, 0.01);
  bind(card, 'XTC threshold', 'xtc_threshold', 0, 1, 0.01);
  bind(card, 'Dynamic temperature range', 'dynatemp_range', 0, 2, 0.01);
  bind(card, 'Dynamic temperature exponent', 'dynatemp_exponent', 0, 2, 0.01);
  return root;
}

export function renderPenaltiesSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';
  const card = sectionCard(t('Repetition penalties'));
  root.appendChild(card);
  bind(card, 'Repeat last N', 'repeat_last_n', 0, 2048, 1);
  bind(card, 'Repeat penalty', 'repeat_penalty', 0, 2, 0.01);
  bind(card, 'Presence penalty', 'presence_penalty', -2, 2, 0.01);
  bind(card, 'Frequency penalty', 'frequency_penalty', -2, 2, 0.01);
  bind(card, 'DRY multiplier', 'dry_multiplier', 0, 2, 0.01);
  bind(card, 'DRY base', 'dry_base', 0, 2, 0.01);
  bind(card, 'DRY allowed length', 'dry_allowed_length', 0, 100, 1);
  bind(card, 'DRY penalty last N', 'dry_penalty_last_n', -1, 2048, 1);
  return root;
}
