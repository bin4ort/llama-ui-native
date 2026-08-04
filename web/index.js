/**
 * index.js — Phase 1 entry: boot the kernel, mount the shell, register the
 * base routes, and render the chat empty state + settings placeholder.
 */
import * as kernel from './kernel/index.js';

const { router, theme, settings, i18n, presets, permissions, log, mountToasts, toast } = kernel;

/* Define globals for the version stamp */
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
    shell.mountShell(document.getElementById('sidebar'), document.getElementById('page-root'));

    router.register(/^\/(?:chat\/new)?$/, () => shell.renderChatEmpty());
    router.register(/^\/chat\/(.+)$/, () => shell.renderChatEmpty());
    router.register(/^\/search$/, () => placeholder('Search'));
    router.register(/^\/mcp-servers$/, () => placeholder('MCP Servers'));
    router.register(/^\/settings\/(.+)$/, () => placeholder('Settings'));

    router.init();

    log.info('LLMUI-SYS-001', 'boot: kernel initialized', { lang: i18n.getLang() });
  } catch (err) {
    log.error('LLMUI-SYS-000', 'boot: entry script failed', err.message, err);
    document.getElementById('page-root').textContent =
      'Boot failed: ' + (err?.message ?? String(err));
  }
}

function placeholder(name) {
  const el = document.createElement('div');
  el.className = 'flex h-full items-center justify-center p-8 text-muted-foreground';
  el.textContent = `${name} — Agent B vertical (Phase 2)`;
  return el;
}

boot();
