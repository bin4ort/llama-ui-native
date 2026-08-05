/**
 * presets.js — Agent B: prompt preset manager + wizard.
 * Library CRUD via kernel presets-store; the wizard generates a detailed
 * system prompt through kernel api.chatCompletion using the professional
 * meta-prompt (port of PRESET_WIZARD_META_PROMPT from the current app).
 */
import { t, log, presets, api } from '../kernel/index.js';
import { button, checkboxField } from './fields.js';

export const WIZARD_META_PROMPT =
  'You are a senior system prompt engineer. Turn the user\'s description into a DETAILED, ' +
  'professional system prompt that works well with local LLMs (llama.cpp, 7B–70B class). ' +
  'The system prompt must be substantially better than what a casual user would write: ' +
  'structured, specific, and grounded in how real experts in that field actually work.\n\n' +
  'REQUIREMENTS FOR THE GENERATED SYSTEM PROMPT:\n' +
  '1. STRUCTURE — Organize it into clear labeled sections when the role is complex: ' +
  '"ROLE", "HOW TO INTERACT / METHOD", "RESPONSE FORMAT", "RULES". Plain text labels are fine; ' +
  'write in second person, imperative mood ("You are…", "Always…", "Never…"). Short, forceful sentences.\n' +
  '2. DEPTH — Specify what a real practitioner would actually do:\n' +
  '   - the concrete methods, techniques or frameworks of the field;\n' +
  '   - the exact session behavior: how to open, whether to ask one clarifying question at a time, ' +
  '   when to challenge or summarize, how to end a session;\n' +
  '   - the expected answer format per message: typical length, when to use numbered lists, ' +
  '   step-by-step reasoning, when to ask follow-up questions, when to give a concrete example;\n' +
  '   - a short concrete example of a good response when it clarifies the format.\n' +
  '3. VOICE — Write like a skilled human expert: natural, specific, direct. Ban robotic phrases ' +
  '("As an AI", "I\'m here to help", "Let\'s get started", "Feel free to ask", "This will help you ' +
  'achieve your goals"). No corporate fluff, no generic filler.\n' +
  '4. REAL HELPFULNESS — The persona must ACT AS the expert the user asked for. Never write ' +
  'cop-outs like "consider seeking professional help", "consult a specialist" or "I am not ' +
  'qualified". The only exception: genuine safety-critical situations — then give a concise, ' +
  'humane safety note and continue helping.\n' +
  '5. RULES — Include a short "RULES" section listing what the persona must NOT do: no generic ' +
  'advice, no unsolicited listicles, no invented studies or statistics, no substance-free praise, ' +
  'no role ambiguity.\n\n' +
  'The resulting system prompt must be 150–400 words (longer for elaborate roles). Use the full ' +
  'space for substance: concrete techniques, specific formats, explicit behavior rules.\n\n' +
  'OUTPUT — your ENTIRE reply must be one valid JSON object with exactly two keys, and nothing ' +
  'else (no code fences, no commentary, no section labels):\n' +
  '{"description": "<one-line description for a picker list, max 12 words>", "content": "<the full system prompt text>"}';

function parseGenerated(raw) {
  let cleaned = raw.trim().replace(/^```(?:json)?\s*/i, '').replace(/```\s*$/, '');
  const start = cleaned.indexOf('{');
  const end = cleaned.lastIndexOf('}');
  if (start !== -1 && end > start) cleaned = cleaned.slice(start, end + 1);
  try {
    const obj = JSON.parse(cleaned);
    if (obj && typeof obj.content === 'string') {
      return { description: typeof obj.description === 'string' ? obj.description.trim() : '', content: obj.content.trim() };
    }
  } catch {
    /* fall through */
  }
  return { description: '', content: cleaned };
}

export function renderPresetsPage(container) {
  const root = document.createElement('div');
  root.className = 'mx-auto w-full max-w-3xl p-6';
  const h = document.createElement('h2');
  h.className = 'text-lg font-semibold mb-1';
  h.textContent = t('Prompt presets');
  root.appendChild(h);
  const sub = document.createElement('p');
  sub.className = 'text-sm text-muted-foreground mb-4';
  sub.textContent = t('Reusable system prompts (personas). Apply them from the chat bar or let the model switch via change_preset.');
  root.appendChild(sub);

  const search = document.createElement('input');
  search.placeholder = t('Search presets...');
  search.className = 'mb-4 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
  root.appendChild(search);

  const wizardBtn = button(t('Create with wizard…'), () => openWizard(renderList), 'outline');
  root.appendChild(wizardBtn);

  const list = document.createElement('div');
  list.className = 'mt-4 space-y-2';
  root.appendChild(list);

  const editBuffers = new Map();

  function renderList() {
    const q = search.value.trim().toLowerCase();
    const items = presets.getPresets().filter(
      (p) => !q || p.name.toLowerCase().includes(q) || (p.description ?? '').toLowerCase().includes(q)
    );
    list.replaceChildren();
    if (items.length === 0) {
      const p = document.createElement('p');
      p.className = 'py-6 text-center text-sm text-muted-foreground';
      p.textContent = presets.getPresets().length === 0 ? t('No presets yet') : t('No matches');
      list.appendChild(p);
      return;
    }
    for (const preset of items) {
      const editing = editBuffers.has(preset.id);
      const card = document.createElement('div');
      card.className = 'rounded-md border border-border/40 bg-card p-3';
      if (!editing) {
        const row = document.createElement('div');
        row.className = 'flex items-center gap-2';
        const star = document.createElement('button');
        star.type = 'button';
        star.className = 'shrink-0 p-1 text-muted-foreground hover:text-foreground';
        star.title = preset.favorite ? t('Remove from favorites') : t('Add to favorites');
        star.textContent = preset.favorite ? '★' : '☆';
        star.addEventListener('click', () => {
          presets.toggleFavorite(preset.id);
          renderList();
        });
        row.appendChild(star);
        const text = document.createElement('div');
        text.className = 'min-w-0 flex-1';
        const name = document.createElement('div');
        name.className = 'truncate text-sm font-medium';
        name.textContent = preset.name;
        text.appendChild(name);
        if (preset.description) {
          const d = document.createElement('div');
          d.className = 'truncate text-xs text-muted-foreground';
          d.textContent = preset.description;
          text.appendChild(d);
        }
        row.appendChild(text);
        row.appendChild(button(t('Edit'), () => {
          editBuffers.set(preset.id, { name: preset.name, description: preset.description ?? '', content: preset.content });
          renderList();
        }, 'ghost'));
        row.appendChild(button(t('Delete'), () => {
          presets.removePreset(preset.id);
          renderList();
        }, 'ghost'));
        card.appendChild(row);
      } else {
        const buf = editBuffers.get(preset.id);
        const makeInput = (value, onInput) => {
          const i = document.createElement('input');
          i.className = 'mb-2 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
          i.value = value;
          i.addEventListener('input', () => onInput(i.value));
          return i;
        };
        const nameIn = makeInput(buf.name, (v) => (buf.name = v));
        const descIn = makeInput(buf.description, (v) => (buf.description = v));
        const content = document.createElement('textarea');
        content.className = 'mb-2 w-full rounded-md border border-input bg-background p-2 font-mono text-xs';
        content.rows = 8;
        content.value = buf.content;
        content.addEventListener('input', () => (buf.content = content.value));
        card.appendChild(nameIn);
        card.appendChild(descIn);
        card.appendChild(content);
        const row = document.createElement('div');
        row.className = 'flex items-center gap-2';
        row.appendChild(button(t('Save'), () => {
          presets.updatePreset(preset.id, buf);
          editBuffers.delete(preset.id);
          renderList();
        }));
        row.appendChild(button(t('Cancel'), () => {
          editBuffers.delete(preset.id);
          renderList();
        }, 'outline'));
        card.appendChild(row);
      }
      list.appendChild(card);
    }
  }
  search.addEventListener('input', renderList);
  renderList();

  container.replaceChildren(root);
}

function openWizard(onSaved) {
  const overlay = document.createElement('div');
  overlay.className =
    'fixed inset-0 z-50 flex items-center justify-center bg-black/50 p-4';
  const dialog = document.createElement('div');
  dialog.className = 'w-full max-w-xl rounded-lg border border-border bg-card p-5 shadow-xl max-h-[85vh] overflow-y-auto';
  const h = document.createElement('h3');
  h.className = 'text-base font-semibold mb-1';
  h.textContent = t('Create a prompt preset');
  dialog.appendChild(h);
  const sub = document.createElement('p');
  sub.className = 'mb-4 text-sm text-muted-foreground';
  sub.textContent = t('Describe the personality or expert role — the model drafts the system prompt for you to review.');
  dialog.appendChild(sub);

  const request = document.createElement('textarea');
  request.rows = 3;
  request.className = 'mb-3 w-full rounded-md border border-input bg-background p-2 text-sm';
  request.placeholder = t('A psychologist who helps me untangle a decision…');
  dialog.appendChild(request);

  const status = document.createElement('p');
  status.className = 'mb-3 text-sm text-destructive';
  dialog.appendChild(status);

  const generateBtn = button(t('Generate'), () => {
    const req = request.value.trim();
    if (!req) return;
    generateBtn.disabled = true;
    generateBtn.textContent = '…';
    status.textContent = '';
    api
      .chatCompletion([{ role: 'user', content: `${WIZARD_META_PROMPT}\n\n${req}` }], {
        temperature: 0.4,
        max_tokens: 1200
      })
      .then((raw) => {
        const parsed = parseGenerated(raw);
        draftPanel.style.display = '';
        nameIn.value = req.slice(0, 50);
        descIn.value = parsed.description;
        contentIn.value = parsed.content;
      })
      .catch((err) => {
        status.textContent = `${t('Generation failed')} (${err?.code ?? 'LLMUI-PRS-001'})`;
        log.error('LLMUI-PRS-001', 'presets: wizard generation failed', err?.message ?? String(err));
      })
      .finally(() => {
        generateBtn.disabled = false;
        generateBtn.textContent = t('Generate');
      });
  });
  dialog.appendChild(generateBtn);

  const draftPanel = document.createElement('div');
  draftPanel.style.display = 'none';
  const nameIn = document.createElement('input');
  nameIn.className = 'mb-2 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
  nameIn.placeholder = t('Preset name');
  const descIn = document.createElement('input');
  descIn.className = 'mb-2 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
  descIn.placeholder = t('Short description (shown in the picker)');
  const contentIn = document.createElement('textarea');
  contentIn.className = 'mb-2 w-full rounded-md border border-input bg-background p-2 font-mono text-xs';
  contentIn.rows = 12;
  draftPanel.appendChild(nameIn);
  draftPanel.appendChild(descIn);
  draftPanel.appendChild(contentIn);
  const saveRow = document.createElement('div');
  saveRow.className = 'flex items-center gap-2';
  saveRow.appendChild(button(t('Save preset'), () => {
    if (!contentIn.value.trim() || !nameIn.value.trim()) return;
    presets.addPreset({
      name: nameIn.value.trim(),
      description: descIn.value.trim() || request.value.trim() || undefined,
      content: contentIn.value.trim()
    });
    close();
    onSaved?.();
  }));
  saveRow.appendChild(button(t('Cancel'), close, 'outline'));
  draftPanel.appendChild(saveRow);
  dialog.appendChild(draftPanel);

  const cancelBtn = button(t('Close'), close, 'ghost');
  const foot = document.createElement('div');
  foot.className = 'mt-3 flex justify-end';
  foot.appendChild(cancelBtn);
  dialog.appendChild(foot);

  function close() {
    overlay.remove();
  }
  overlay.addEventListener('click', (e) => {
    if (e.target === overlay) close();
  });
  overlay.appendChild(dialog);
  document.body.appendChild(overlay);
}
