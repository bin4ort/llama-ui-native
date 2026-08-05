/**
 * developer.js — Agent B: Developer settings section.
 * API key, server endpoint, JS sandbox flags, and the log-level slider
 * (0 Errors … 4 Trace) with a live debug-log view + "Copy debug log".
 */
import { t, log, getLogLevel, getRingEntries, updateConfig, resetConfig, config } from '../../kernel/index.js';
import { checkboxField, textField, sectionCard, button } from '../fields.js';

export const LOG_LEVEL_LABELS = ['Errors', 'Warnings', 'Info', 'Debug', 'Trace'];

export function renderDeveloperSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  root.appendChild(sectionCard(t('Server')));
  root.lastChild.appendChild(
    textField(t('API Key'), config().apiKey, t('Optional key sent to the model server.'), (v) =>
      updateConfig({ apiKey: v })
    )
  );
  root.lastChild.appendChild(
    textField(t('Server endpoint'), config().serverEndpoint, 'http://localhost:8080', (v) =>
      updateConfig({ serverEndpoint: v })
    )
  );

  root.appendChild(sectionCard(t('JavaScript sandbox')));
  root.lastChild.appendChild(
    checkboxField(
      t('Enable JS sandbox'),
      config().jsSandboxEnabled,
      t('Run frontend JavaScript tools in a sandboxed worker.'),
      (v) => updateConfig({ jsSandboxEnabled: v })
    )
  );
  root.lastChild.appendChild(
    checkboxField(
      t('Symbolic math'),
      config().symbolicMathEnabled,
      'nerdamer symbolic computation for the calculator tool.',
      (v) => updateConfig({ symbolicMathEnabled: v })
    )
  );

  root.appendChild(sectionCard(t('Logging')));
  const levelCard = root.lastChild;

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
  const applyLevel = (v) => {
    const n = Number(v);
    updateConfig({ logLevel: n });
    levelLabel.textContent = LOG_LEVEL_LABELS[n] ?? String(n);
  };
  slider.addEventListener('input', () => applyLevel(slider.value));
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
  levelCard.appendChild(levelWrap);

  // Live debug log view (kernel ring buffer)
  const logBox = document.createElement('pre');
  logBox.className =
    'max-h-56 overflow-auto rounded-md border border-border/60 bg-muted/40 p-2 font-mono text-[11px] leading-relaxed whitespace-pre-wrap';
  const renderLog = () => {
    const entries = getRingEntries();
    logBox.textContent = entries
      .map((e) => `${e.ts} | ${e.level} | ${e.code} | ${e.message}`)
      .join('\n');
    logBox.scrollTop = logBox.scrollHeight;
  };
  renderLog();
  const refresh = setInterval(renderLog, 1000);
  levelCard.addEventListener('remove', () => clearInterval(refresh), { once: true });

  const row = document.createElement('div');
  row.className = 'space-y-1.5 py-3 first:pt-0 last:pb-0';
  const logLab = document.createElement('label');
  logLab.className = 'block text-sm font-medium';
  logLab.textContent = t('Debug log');
  row.appendChild(logLab);
  row.appendChild(logBox);

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
  row.appendChild(actions);
  levelCard.appendChild(row);

  root.appendChild(sectionCard(t('Danger zone')));
  const danger = root.lastChild;
  danger.appendChild(
    checkboxField(
      t('Enable Web UI'),
      config().enableWebUi,
      t('Expose the built-in web UI (native window remains the primary interface).'),
      (v) => updateConfig({ enableWebUi: v })
    )
  );
  const resetRow = document.createElement('div');
  resetRow.className = 'flex items-center gap-2 py-3';
  resetRow.appendChild(button(t('Reset settings'), () => {
    // eslint-disable-next-line no-restricted-globals
    if (confirm(t('Reset all settings to defaults? This cannot be undone.'))) {
      resetConfig();
      location.reload();
    }
  }, 'outline'));
  danger.appendChild(resetRow);

  return root;
}
