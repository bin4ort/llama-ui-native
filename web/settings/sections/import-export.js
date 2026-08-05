/**
 * import-export.js — Agent B: Import/Export section.
 * Settings export/import (JSON file) and conversation export/import as a
 * .zip archive (fflate), same payload shapes as the current app.
 */
import { t, log, config, updateConfig, db } from '../../kernel/index.js';
import { button, sectionCard } from '../fields.js';
import { zipSync, unzipSync, strToU8, strFromU8 } from 'fflate';

function download(name, blob) {
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = name;
  a.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
}

export function renderImportExportSection() {
  const root = document.createElement('div');
  root.className = 'space-y-4';

  // ---- Settings ----
  root.appendChild(sectionCard(t('Settings')));
  const settingsCard = root.lastChild;
  const settingsRow = document.createElement('div');
  settingsRow.className = 'flex items-center gap-2 py-2';
  settingsRow.appendChild(
    button(t('Export settings'), () => {
      try {
        download(
          `llama-ui-settings-${new Date().toISOString().slice(0, 10)}.json`,
          new Blob([JSON.stringify(config(), null, 2)], { type: 'application/json' })
        );
        log.info('LLMUI-CFG-005', 'settings exported');
      } catch (err) {
        log.error('LLMUI-CFG-005', 'settings export failed', String(err));
      }
    })
  );
  settingsRow.appendChild(
    button(t('Import settings'), () => filePicker('application/json', async (text) => {
      try {
        const parsed = JSON.parse(text);
        if (typeof parsed !== 'object' || Array.isArray(parsed)) throw new Error('bad payload');
        updateConfig(parsed);
        log.info('LLMUI-CFG-006', 'settings imported', Object.keys(parsed).length + ' keys');
      } catch (err) {
        log.error('LLMUI-CFG-004', 'settings import rejected', err?.message ?? String(err));
        alert(t('Import failed: not a valid settings file.'));
      }
    }), 'outline')
  );
  settingsCard.appendChild(settingsRow);

  // ---- Conversations ----
  root.appendChild(sectionCard(t('Conversations')));
  const convCard = root.lastChild;
  const convRow = document.createElement('div');
  convRow.className = 'flex items-center gap-2 py-2';
  convRow.appendChild(
    button(t('Export conversations'), async () => {
      try {
        const convs = await db.listConversations();
        const files = {};
        for (const conv of convs) {
          const messages = await db.getMessagesByConversation(conv.id);
          files[`${conv.id}.json`] = strToU8(JSON.stringify({ conv, messages }, null, 2));
        }
        if (Object.keys(files).length === 0) {
          files['empty.txt'] = strToU8('no conversations');
        }
        download(
          `llama-ui-conversations-${new Date().toISOString().slice(0, 10)}.zip`,
          new Blob([zipSync(files)], { type: 'application/zip' })
        );
        log.info('LLMUI-CFG-005', 'conversations exported', Object.keys(files).length + ' convs');
      } catch (err) {
        log.error('LLMUI-CFG-005', 'conversations export failed', String(err));
      }
    })
  );
  convRow.appendChild(
    button(t('Import conversations'), () => filePicker('.zip,application/zip', async (buf) => {
      try {
        const files = unzipSync(new Uint8Array(buf));
        let imported = 0;
        for (const [name, data] of Object.entries(files)) {
          if (!name.endsWith('.json')) continue;
          const { conv, messages } = JSON.parse(strFromU8(data));
          if (!conv?.id || !Array.isArray(messages)) continue;
          const exists = await db.getConversation(conv.id);
          if (exists) continue;
          await db.addConversation(conv);
          for (const m of messages) await db.addMessage(m);
          imported++;
        }
        log.info('LLMUI-CFG-006', 'conversations imported', imported + ' convs');
        alert(t('Imported') + ` ${imported} ${t('conversations')}.`);
      } catch (err) {
        log.error('LLMUI-CFG-004', 'conversations import rejected', err?.message ?? String(err));
        alert(t('Import failed: not a valid conversations archive.'));
      }
    }), 'outline')
  );
  convCard.appendChild(convRow);

  return root;
}

function filePicker(accept, onRead) {
  const input = document.createElement('input');
  input.type = 'file';
  input.accept = accept;
  input.addEventListener('change', () => {
    const file = input.files?.[0];
    if (!file) return;
    if (accept.includes('json')) {
      file.text().then(onRead);
    } else {
      file.arrayBuffer().then(onRead);
    }
  });
  input.click();
}
