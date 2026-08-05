/**
 * index.js — entry: boot kernel, mount shell, register routes.
 * Phase 2 (Agent A): chat, search live; settings/MCP remain Agent B stubs.
 */
import * as kernel from './kernel/index.js';

const { router, theme, settings, i18n, presets, permissions, log, mountToasts } = kernel;

globalThis.APP_VERSION = '0.4.3';
globalThis.APP_BUILD = '0x07D21';

async function boot() {
  try {
    const cfg = settings.loadConfig();
    i18n.setLang(cfg.language ?? 'en');
    theme.initTheme(cfg.theme ?? 'system');
    permissions.loadPermissions();
    presets.seedBuiltinPresets();

    mountToasts(document.getElementById('toast-host'));

    const shell = await import('./app/shell.js');

    router.register(/^\/(?:chat\/new)?$/, () => shell.renderChat(['new']));
    router.register(/^\/chat\/(.+)$/, (params) => shell.renderChat(params));
    router.register(/^\/search$/, () => {
      const el = document.createElement('div');
      el.className = 'h-full';
      shell.renderSearch().then((node) => el.replaceChildren(node));
      return el;
    });
    router.register(/^\/mcp-servers$/, () => {
      const el = document.createElement('div');
      el.className = 'h-full';
      import('./settings/index.js').then((m) => m.renderMcpRoute(el));
      return el;
    });
    router.register(/^\/settings\/presets$/, () => {
      const el = document.createElement('div');
      el.className = 'h-full';
      import('./settings/index.js').then((m) => m.renderPresetsRoute(el));
      return el;
    });
    router.register(/^\/settings\/(.+)$/, (params) => {
      const el = document.createElement('div');
      el.className = 'h-full';
      import('./settings/index.js').then((m) => m.renderSettingsPage(el, params));
      return el;
    });

    shell.mountShell(document.getElementById('sidebar'), document.getElementById('page-root'));
    router.init();

    log.info('LLMUI-SYS-001', 'boot: kernel initialized', { lang: i18n.getLang() });
  } catch (err) {
    log.error('LLMUI-SYS-000', 'boot: entry script failed', err?.message ?? String(err));
    document.getElementById('page-root').textContent =
      'Boot failed: ' + (err?.message ?? String(err));
  }
}

boot();
