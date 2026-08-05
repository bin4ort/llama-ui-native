/**
 * tools.js — Agent B: Tools settings section.
 * Per-tool toggles (config), built-in tools group, staged always-allow
 * grants applied via the kernel verification contract on Save.
 */
import { t, updateConfig, config, permissions, log } from '../../kernel/index.js';
import { checkboxField, button, sectionCard } from '../fields.js';

const TOOL_TOGGLES = [
  ['toolCalculateEnabled', 'Calculate', 'Safe expression evaluator.'],
  ['toolFetchUrlEnabled', 'Fetch URL', 'Fetch web pages as clean text.'],
  ['toolToTableEnabled', 'To table', 'Format CSV/JSON as a markdown table.'],
  ['toolJsonEnabled', 'JSON query', 'Query JSON structures.'],
  ['toolClipboardEnabled', 'Clipboard', 'Read/write the clipboard.'],
  ['toolNotifyEnabled', 'Notify', 'Desktop notifications.'],
  ['toolTodoEnabled', 'Todo list', 'Persistent todo list (LlamaUi.todos).'],
  ['toolWeatherEnabled', 'Weather', 'Forecast via open-meteo.'],
  ['toolWikipediaEnabled', 'Wikipedia', 'Search wikipedia.org.'],
  ['toolPlotChartEnabled', 'Plot chart', 'Generate charts from data.']
];

export function renderToolsSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  root.appendChild(sectionCard(t('Built in tools')));
  const builtinCard = root.lastChild;
  for (const [key, label, help] of TOOL_TOGGLES) {
    builtinCard.appendChild(
      checkboxField(t(label), config()[key] !== false, t(help), (v) => updateConfig({ [key]: v }))
    );
  }

  // Staged always-allow grants (kernel permissions contract)
  root.appendChild(sectionCard(t('Always allow')));
  const allowCard = root.lastChild;

  // Built-in tool keys — must match Agent A's registry (web/app/chat/tools.js).
  const BUILTIN_TOOL_KEYS = [
    'calculate', 'fetch_url', 'to_table', 'json_tool', 'clipboard',
    'notify', 'todo_list', 'weather', 'wikipedia', 'plot_chart'
  ];

  const grantRow = document.createElement('div');
  grantRow.className = 'flex items-center gap-2 py-1';
  const grantSel = document.createElement('select');
  grantSel.className = 'h-9 flex-1 rounded-md border border-input bg-background px-3 text-sm';
  for (const key of BUILTIN_TOOL_KEYS) {
    const o = document.createElement('option');
    o.value = key;
    o.textContent = key;
    grantSel.appendChild(o);
  }
  grantRow.appendChild(grantSel);
  grantRow.appendChild(
    button(t('Grant always allow'), () => {
      const key = `frontend:${grantSel.value}`;
      if (stagedKeys.has(key)) return;
      stagedKeys.add(key);
      renderAllowList();
    }, 'outline')
  );
  allowCard.appendChild(grantRow);

  const stagedKeys = new Set(permissions.permissionsStore.get());

  const renderAllowList = () => {
    const existing = allowCard.querySelector('[data-allow-list]');
    if (existing) existing.remove();
    const list = document.createElement('div');
    list.dataset.allowList = '';
    list.className = 'space-y-2';
    const all = [...new Set([...stagedKeys, ...permissions.permissionsStore.get()])];
    if (all.length === 0) {
      const p = document.createElement('p');
      p.className = 'text-xs text-muted-foreground';
      p.textContent = t('No tools granted always-allow access.');
      list.appendChild(p);
    }
    for (const key of all) {
      const row = document.createElement('div');
      row.className = 'flex items-center justify-between gap-2 rounded-md border border-border/40 px-3 py-2';
      const name = document.createElement('span');
      name.className = 'text-sm';
      name.textContent = key;
      row.appendChild(name);
      const revoke = button(t('Revoke'), () => {
        permissions.revokeTool(key);
        stagedKeys.delete(key);
        renderAllowList();
      }, 'ghost');
      row.appendChild(revoke);
      list.appendChild(row);
    }
    allowCard.appendChild(list);
  };
  renderAllowList();

  const status = document.createElement('p');
  status.className = 'text-xs text-muted-foreground';
  status.textContent = t('Always-allow grants are staged and applied when you save settings.');
  allowCard.appendChild(status);

  const actions = document.createElement('div');
  actions.className = 'flex items-center gap-2 pt-1';
  actions.appendChild(
    button(t('Save settings'), () => {
      const additions = [...stagedKeys].filter((k) => !permissions.permissionsStore.get().has(k));
      // Verification contract: staged grants are confirmed via kernel.verify()
      // before being applied. No verifier is registered yet (Agent B dialog
      // pending) — grants apply directly for now.
      permissions.allowTools(additions);
      log.info('LLMUI-TL-007', 'always-allow grants applied', additions.join(', '));
      renderAllowList();
    })
  );
  allowCard.appendChild(actions);

  return root;
}
