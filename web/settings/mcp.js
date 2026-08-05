/**
 * mcp.js — Agent B: MCP Servers page.
 * CRUD over config.mcpServers (JSON string, same key as the current app) +
 * per-server enabled toggle and a connection test via the kernel api.
 */
import { t, log, config, updateConfig, api } from '../kernel/index.js';
import { button, textField, checkboxField, sectionCard } from './fields.js';

function parseServers() {
  try {
    const raw = config().mcpServers || '[]';
    const parsed = JSON.parse(raw);
    return Array.isArray(parsed) ? parsed : [];
  } catch (err) {
    log.warn('LLMUI-CFG-009', 'mcp: servers payload invalid', String(err));
    return [];
  }
}

function persistServers(list) {
  updateConfig({ mcpServers: JSON.stringify(list, null, 2) });
}

export function renderMcpServersPage(container) {
  const root = document.createElement('div');
  root.className = 'mx-auto w-full max-w-3xl p-6';
  const h = document.createElement('h2');
  h.className = 'text-lg font-semibold mb-4';
  h.textContent = t('MCP Servers');
  root.appendChild(h);

  const renderList = () => {
    const old = root.querySelector('[data-mcp-list]');
    if (old) old.remove();
    const list = document.createElement('div');
    list.dataset.mcpList = '';
    list.className = 'space-y-3';
    const servers = parseServers();
    if (servers.length === 0) {
      const p = document.createElement('p');
      p.className = 'text-sm text-muted-foreground';
      p.textContent = t('No MCP servers configured yet.');
      list.appendChild(p);
    }
    for (const s of servers) {
      const card = sectionCard(s.name || s.id || 'MCP server');
      const urlP = document.createElement('p');
      urlP.className = 'text-xs text-muted-foreground break-all';
      urlP.textContent = s.url ?? '';
      card.appendChild(urlP);
      card.appendChild(
        checkboxField(t('Enabled'), s.enabled !== false, '', (v) => {
          persistServers(servers.map((x) => (x === s ? { ...x, enabled: v } : x)));
        })
      );
      const row = document.createElement('div');
      row.className = 'flex items-center gap-2 py-2';
      row.appendChild(
        button(t('Test connection'), async () => {
          try {
            const res = await fetch(`${(s.url ?? '').replace(/\/+$/, '')}/`, {
              signal: AbortSignal.timeout(5000)
            });
            log.info('LLMUI-MCP-008', 'mcp: connection test ok', `${s.url} -> ${res.status}`);
            alert(`${s.name ?? s.id}: HTTP ${res.status}`);
          } catch (err) {
            log.error('LLMUI-MCP-008', 'mcp: connection test failed', String(err));
            alert(t('Connection failed'));
          }
        }, 'outline')
      );
      row.appendChild(
        button(t('Delete'), () => {
          persistServers(servers.filter((x) => x !== s));
          renderList();
        }, 'ghost')
      );
      card.appendChild(row);
      list.appendChild(card);
    }
    root.appendChild(list);
  };
  renderList();

  // Add form
  const addCard = sectionCard(t('Add server'));
  const name = document.createElement('input');
  name.placeholder = 'Name';
  name.className = 'h-9 rounded-md border border-input bg-background px-3 text-sm mb-2';
  const url = document.createElement('input');
  url.placeholder = 'http://localhost:3001/mcp';
  url.className = 'h-9 rounded-md border border-input bg-background px-3 text-sm mb-2 w-full';
  addCard.appendChild(name);
  addCard.appendChild(url);
  addCard.appendChild(
    button(t('Add'), () => {
      const servers = parseServers();
      servers.push({
        id: crypto.randomUUID(),
        name: name.value.trim() || 'MCP server',
        url: url.value.trim(),
        enabled: true
      });
      persistServers(servers);
      name.value = '';
      url.value = '';
      renderList();
    })
  );
  root.appendChild(addCard);

  container.replaceChildren(root);
}
