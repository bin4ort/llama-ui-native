/**
 * presets-wizard.js — Agent B: preset wizard dialog (shared).
 * Used by the preset manager page and the full picker dialog. Generates a
 * detailed system prompt via kernel api.chatCompletion with the professional
 * meta-prompt; the draft is always shown for review, never auto-saved.
 */
import { t, log, api, presets, showModal, button } from '../kernel/index.js';

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
      return {
        description: typeof obj.description === 'string' ? obj.description.trim() : '',
        content: obj.content.trim()
      };
    }
  } catch {
    /* fall through */
  }
  return { description: '', content: cleaned };
}

/**
 * Open the wizard as a modal. onSaved(preset) fires after the user saves a
 * generated preset (never auto-saves).
 */
export function openWizardDialog(onSaved) {
  const body = document.createElement('div');
  body.className = 'space-y-3';

  const request = document.createElement('textarea');
  request.rows = 3;
  request.className = 'w-full rounded-md border border-input bg-background p-2 text-sm';
  request.placeholder = t('A psychologist who helps me untangle a decision…');
  body.appendChild(request);

  const status = document.createElement('p');
  status.className = 'text-sm text-destructive';
  body.appendChild(status);

  const draftPanel = document.createElement('div');
  draftPanel.style.display = 'none';
  const nameIn = document.createElement('input');
  nameIn.className = 'h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
  nameIn.placeholder = t('Preset name');
  const descIn = document.createElement('input');
  descIn.className = 'mt-2 h-9 w-full rounded-md border border-input bg-background px-3 text-sm';
  descIn.placeholder = t('Short description (shown in the picker)');
  const contentIn = document.createElement('textarea');
  contentIn.className = 'mt-2 w-full rounded-md border border-input bg-background p-2 font-mono text-xs';
  contentIn.rows = 12;
  draftPanel.appendChild(nameIn);
  draftPanel.appendChild(descIn);
  draftPanel.appendChild(contentIn);

  const saveRow = document.createElement('div');
  saveRow.className = 'flex items-center justify-end gap-2 pt-1';
  const saveBtn = button(t('Save preset'), () => {
    if (!contentIn.value.trim() || !nameIn.value.trim()) return;
    const preset = presets.addPreset({
      name: nameIn.value.trim(),
      description: descIn.value.trim() || request.value.trim() || undefined,
      content: contentIn.value.trim()
    });
    handle.close();
    onSaved?.(preset);
  });
  saveRow.appendChild(saveBtn);
  draftPanel.appendChild(saveRow);
  body.appendChild(draftPanel);

  const genRow = document.createElement('div');
  genRow.className = 'flex items-center gap-2';
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
  genRow.appendChild(generateBtn);
  body.appendChild(genRow);

  const handle = showModal({
    title: t('Create a prompt preset'),
    content: () => body
  });
}

/** Parse helper re-exported for tests. */
export { parseGenerated };
