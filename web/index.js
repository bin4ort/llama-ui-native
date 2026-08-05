/**
 * index.js — entry: boot kernel, mount shell, register routes.
 * Phase 2 (Agent A): chat, search live; settings/MCP remain Agent B stubs.
 */
import * as kernel from './kernel/index.js';

const { router, theme, settings, i18n, presets, permissions, log, mountToasts, mountModalHost } = kernel;

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
    mountModalHost(document.getElementById('modal-host'));

    // PWA (Agent B): service worker — cache-first for hashed assets,
    // network-first for the shell. Skipped in the native window.
    if ('serviceWorker' in navigator && !new URLSearchParams(window.location.search).has('native')) {
      navigator.serviceWorker.register('/sw.js').catch((err) => {
        log.warn('LLMUI-PWA-000', 'pwa: service worker registration failed', String(err));
      });
    }

    // Agent B dialogs: real verification dialog + preset picker. Loaded
    // lazily so the settings tree stays out of the chat critical path.
    import('./settings/verify-dialog.js').then((m) => m.registerVerificationDialog());
    import('./settings/presets-picker.js').then((m) => m.registerPresetPicker());

    // e2e hook: ?autoapprove=1 resolves every tool permission automatically
    if (new URLSearchParams(window.location.search).has('autoapprove')) {
      permissions.registerVerifier(() => Promise.resolve(true));
    }
    // e2e hook: ?test=1 exposes the kernel facade for headless tests
    if (new URLSearchParams(window.location.search).has('test')) {
      globalThis.__kernel = kernel;
    }

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
