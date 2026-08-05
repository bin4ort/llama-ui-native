/**
 * developer.js — Agent B: Developer settings section.
 * Registry-driven (8 fields, old order) + the log-level slider with live
 * debug-log view and Copy, and the reset-to-default action.
 */
import { t, log, getLogLevel, getRingEntries, updateConfig, resetConfig, config } from '../../kernel/index.js';
import { registrySection, button } from '../fields.js';
import { FIELDS } from '../registry.js';

export const LOG_LEVEL_LABELS = ['Errors', 'Warnings', 'Info', 'Debug', 'Trace'];

export function renderDeveloperSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  root.appendChild(
    registrySection(FIELDS.developer, { config, updateConfig, translate: (s) => t(s) })
  );

  // ---- Logging (log-level slider + debug log) ----
  const logging = document.createElement('div');
  logging.className = 'space-y-6';

  const levelWrap = document.createElement('div');
  levelWrap.className = 'space-y-1.5 py-3 first:pt-0 last:pb-0';
  const head = document.createElement('div');
  head.className = 'flex items-center justify-between gap-2';
  const lab = document.createElement('label');
  lab.className = 'text-sm font-medium';
  lab.textContent = t('Log level');
  head.appendChild(lab);
  const levelLabel = document.createElement('span');
  levelLabel.className = 'text-xs tabular-nums text-muted-foreground';
  levelLabel.textContent = LOG_LEVEL_LABELS[getLogLevel()] ?? String(getLogLevel());
  head.appendChild(levelLabel);
  levelWrap.appendChild(head);

  const slider = document.createElement('input');
  slider.type = 'range';
  slider.min = '0';
  slider.max = '4';
  slider.step = '1';
  slider.value = String(getLogLevel());
  slider.className = 'w-full accent-primary';
  slider.addEventListener('input', () => {
    const n = Number(slider.value);
    updateConfig({ logLevel: n });
    levelLabel.textContent = LOG_LEVEL_LABELS[n] ?? String(n);
  });
  levelWrap.appendChild(slider);

  const ticks = document.createElement('div');
  ticks.className = 'flex justify-between text-[10px] text-muted-foreground';
  LOG_LEVEL_LABELS.forEach((l) => {
    const s = document.createElement('span');
    s.textContent = l;
    ticks.appendChild(s);
  });
  levelWrap.appendChild(ticks);
  const help = document.createElement('p');
  help.className = 'text-xs text-muted-foreground';
  help.textContent = t('Errors only logs critical failures; Trace logs everything without exception.');
  levelWrap.appendChild(help);
  logging.appendChild(levelWrap);

  const logBox = document.createElement('pre');
  logBox.className = 'settings-log max-h-56 overflow-auto rounded-md border border-border/60 bg-muted/40 p-2 font-mono text-[11px] leading-relaxed whitespace-pre-wrap';
  const renderLog = () => {
    const entries = getRingEntries();
    logBox.textContent = entries.map((e) => `${e.ts} | ${e.level} | ${e.code} | ${e.message}`).join('\n');
    logBox.scrollTop = logBox.scrollHeight;
  };
  renderLog();
  const refresh = setInterval(renderLog, 1000);
  root.addEventListener('remove', () => clearInterval(refresh), { once: true });

  const logRow = document.createElement('div');
  logRow.className = 'space-y-1.5 py-3 first:pt-0 last:pb-0';
  const logLab = document.createElement('label');
  logLab.className = 'block text-sm font-medium';
  logLab.textContent = t('Debug log');
  logRow.appendChild(logLab);
  logRow.appendChild(logBox);
  const actions = document.createElement('div');
  actions.className = 'flex items-center gap-2 pt-1';
  actions.appendChild(
    button(t('Copy debug log'), () => {
      navigator.clipboard.writeText(
        getRingEntries().map((e) => `${e.ts} | ${e.level} | ${e.code} | ${e.message}`).join('\n')
      );
      log.info('LLMUI-CFG-010', 'debug log copied to clipboard');
    }, 'outline')
  );
  logRow.appendChild(actions);
  logging.appendChild(logRow);
  root.appendChild(logging);

  // ---- Reset ----
  const resetRow = document.createElement('div');
  resetRow.className = 'flex items-center gap-2 py-3';
  resetRow.appendChild(
    button(t('Reset settings'), () => {
      if (confirm(t('Reset all settings to defaults? This cannot be undone.'))) {
        resetConfig();
        location.reload();
      }
    }, 'outline')
  );
  root.appendChild(resetRow);

  return root;
}
