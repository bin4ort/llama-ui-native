/**
 * chat.js — Agent A: chat state (active conversation, message tree, send +
 * SSE streaming, reasoning/tool-call extraction) over the frozen kernel.
 * Error codes used (never edited): LLMUI-API-*, LLMUI-STR-*, LLMUI-CHAT-*,
 * LLMUI-TL-*.
 */
import * as kernel from '../../kernel/index.js';
const t = kernel.t;
import { streamChatCompletion } from '../../kernel/api.js';
import { runAgenticLoop } from './tools.js';

const { store, db, log, presets } = kernel;

export const conversationsStore = store([]); // flat conversations (sorted by lastModified desc)
export const activeConversationStore = store(null);
export const messagesStore = store([]); // flat messages of the active conversation
export const streamingStore = store(false);
export const streamContentStore = store('');
export const contextStore = store(null); // { used, total } from /slots after a completion

let activeId = null;
let abortController = null;

export function loadConversations() {
  db.listConversations()
    .then((list) => {
      list.sort((a, b) => (b.lastModified ?? 0) - (a.lastModified ?? 0));
      conversationsStore.set(list);
    })
    .catch((err) => log.error('LLMUI-DB-001', 'chat: conversations load failed', String(err)));
}

export async function openConversation(id) {
  activeId = id;
  const conv = await db.getConversation(id);
  activeConversationStore.set(conv);
  const msgs = await db.getMessagesByConversation(id);
  msgs.sort((a, b) => (a.timestamp ?? 0) - (b.timestamp ?? 0));
  messagesStore.set(msgs);
}

export async function newConversation() {
  const conv = {
    id: crypto.randomUUID(),
    name: `Chat ${new Date().toLocaleString()}`,
    lastModified: Date.now(),
    currNode: null,
    pinned: false
  };
  await db.addConversation(conv);
  loadConversations();
  await openConversation(conv.id);
  return conv;
}

export async function deleteConversation(id) {
  await db.deleteConversation(id);
  if (activeId === id) {
    activeId = null;
    activeConversationStore.set(null);
    messagesStore.set([]);
  }
  loadConversations();
}

export async function renameConversation(id, name) {
  await db.updateConversation(id, { name });
  loadConversations();
  if (activeId === id) activeConversationStore.update((c) => (c ? { ...c, name } : c));
}

export async function togglePin(id) {
  const conv = await db.getConversation(id);
  if (!conv) return;
  await db.updateConversation(id, { pinned: !conv.pinned });
  loadConversations();
}

async function persistMessage(message) {
  try {
    await db.addMessage(message);
  } catch (err) {
    log.error('LLMUI-DB-002', 'chat: message add failed', String(err));
    throw err;
  }
}

function buildUserMessage(convId, content, attachments = []) {
  return {
    id: crypto.randomUUID(),
    convId,
    type: 'user',
    role: 'user',
    content,
    attachments,
    parent: null,
    children: [],
    timestamp: Date.now()
  };
}

function buildAssistantMessage(convId) {
  return {
    id: crypto.randomUUID(),
    convId,
    type: 'assistant',
    role: 'assistant',
    content: '',
    parent: null,
    children: [],
    timestamp: Date.now()
  };
}

function resolveSystemPrompt(convId) {
  // persona row (type 'persona') + context rows (type 'system') stack, like today
  const msgs = messagesStore.get();
  const persona = msgs.find((m) => m.type === 'persona' && m.content?.trim());
  const context = msgs.find((m) => m.type === 'system' && m.content?.trim());
  return [persona, context].filter(Boolean).map((m) => ({ role: 'system', content: m.content }));
}

function toApiMessages(msgs) {
  const out = resolveSystemPrompt(activeId);
  for (const m of msgs) {
    if (m.type === 'persona' || m.type === 'system') continue;
    if (m.type === 'tool') {
      out.push({ role: 'tool', tool_call_id: m.toolCallId, content: m.content });
    } else if (m.type === 'assistant' && m.toolCalls?.length) {
      out.push({ role: 'assistant', content: m.content || null, tool_calls: m.toolCalls });
    } else if (m.role === 'user') {
      if (m.attachments?.length) {
        const parts = [{ type: 'text', text: m.content || '' }];
        for (const att of m.attachments) {
          if (att.dataUrl && att.type?.startsWith('image/')) {
            parts.push({ type: 'image_url', image_url: { url: att.dataUrl } });
          }
        }
        const files = m.attachments.filter((a) => !a.type?.startsWith('image/'));
        if (files.length) {
          parts[0].text += `\n\n[Attached: ${files.map((f) => f.name).join(', ')}]`;
        }
        out.push({ role: 'user', content: parts });
      } else {
        out.push({ role: 'user', content: m.content });
      }
    } else if (m.role === 'assistant') {
      out.push({ role: 'assistant', content: m.content });
    }
  }
  return out;
}

/**
 * Apply a persona (preset or default system prompt) to the active
 * conversation: updates the persona row (type 'persona') or creates one at
 * the top of history. Persona + context (type 'system') rows stack when sent.
 */
export async function applyPersona(content) {
  if (!activeId) return;
  const trimmed = String(content ?? '').trim();
  const msgs = messagesStore.get();
  let persona = msgs.find((m) => m.type === 'persona');

  if (persona) {
    if (!trimmed) {
      // empty target: remove the persona row
      await db.deleteMessage(persona.id);
      messagesStore.update((list) => list.filter((m) => m.id !== persona.id));
      return;
    }
    await db.updateMessage(persona.id, { content: trimmed });
    messagesStore.update((list) => list.map((m) => (m.id === persona.id ? { ...m, content: trimmed } : m)));
    return;
  }

  if (!trimmed) return; // nothing to remove, no persona row exists

  const first = msgs[0];
  const personaRow = {
    id: crypto.randomUUID(),
    convId: activeId,
    type: 'persona',
    role: 'system',
    content: trimmed,
    parent: null,
    children: [],
    timestamp: (first?.timestamp ?? Date.now()) - 1 // keep it at the top
  };
  await persistMessage(personaRow);
  messagesStore.update((list) => [personaRow, ...list]);
}

/** "Default" persona: the settings default system message (or none). */
export function applyDefaultPersona() {
  return applyPersona(kernel.config().systemMessage ?? '');
}

/**
 * Fork the active conversation: copy all messages into a new conversation
 * named "<original> (fork)". Returns the new conversation id.
 */
export async function forkConversation() {
  const convs = conversationsStore.get();
  const conv = convs.find((c) => c.id === activeId);
  if (!conv) return null;
  const msgs = messagesStore.get();

  const now = Date.now();
  const newConv = {
    id: crypto.randomUUID(),
    name: `${conv.name ?? conv.title ?? ''} (${t('fork')})`,
    pinned: false,
    createdAt: now,
    lastModified: now
  };
  await db.addConversation(newConv);
  for (const m of msgs) {
    await db.addMessage({ ...m, id: crypto.randomUUID(), convId: newConv.id, children: [] });
  }
  await loadConversations();
  await openConversation(newConv.id);
  return newConv.id;
}

/** Truncate all descendants of a message (the active branch after it). */
async function truncateBranch(convId, messageId) {
  const all = await db.getMessagesByConversation(convId);
  const descendants = [];
  const stack = [...all.filter((m) => m.parent === messageId).map((m) => m.id)];
  const byParent = new Map();
  for (const m of all) {
    if (!byParent.has(m.parent)) byParent.set(m.parent, []);
    byParent.get(m.parent).push(m.id);
  }
  while (stack.length) {
    const id = stack.pop();
    descendants.push(id);
    stack.push(...(byParent.get(id) ?? []));
  }
  for (const id of descendants) {
    try {
      await db.deleteMessage(id);
    } catch (err) {
      log.error('LLMUI-DB-004', 'chat: branch delete failed', String(err));
    }
  }
  return descendants;
}

/** Edit a message in place and truncate its descendants (active-branch edit). */
export async function editMessage(id, content) {
  const msg = messagesStore.get().find((m) => m.id === id);
  if (!msg) return;
  const removed = await truncateBranch(activeId, id);
  await db.updateMessage(id, { content });
  messagesStore.update((msgs) => msgs.filter((m) => !removed.includes(m.id)).map((m) => (m.id === id ? { ...m, content } : m)));
}

/** Delete a message and its descendants. */
export async function deleteMessage(id) {
  const removed = await truncateBranch(activeId, id);
  await db.deleteMessage(id);
  messagesStore.update((msgs) => msgs.filter((m) => m.id !== id && !removed.includes(m.id)));
}

/** Regenerate an assistant message: drop it + descendants, re-run the loop from the prior user turn. */
export async function regenerateMessage(id) {
  const msgs = messagesStore.get();
  const idx = msgs.findIndex((m) => m.id === id);
  if (idx === -1 || msgs[idx].role !== 'assistant') return;
  await deleteMessage(id);
  const prior = [...messagesStore.get()].filter((m) => m.role === 'user').at(-1);
  if (prior) await sendMessage(prior.content);
}

/**
 * Send a user message: persist, then stream the completion.
 * Appends the assistant message with delta content, reasoning and
 * tool-call fields.
 */
export async function sendMessage(content, attachments = []) {
  if (!activeId || !content.trim()) return;
  abortController = new AbortController();

  const userMsg = buildUserMessage(activeId, content.trim(), attachments);
  await persistMessage(userMsg);
  messagesStore.update((msgs) => [...msgs, userMsg]);

  const assistant = buildAssistantMessage(activeId);
  messagesStore.update((msgs) => [...msgs, assistant]);
  streamingStore.set(true);
  streamContentStore.set('');

  const apiMessages = toApiMessages(messagesStore.get().filter((m) => m.id !== assistant.id));

  try {
    const result = await runAgenticLoop({
      messages: apiMessages,
      conversationId: activeId,
      options: {
        temperature: Number(kernel.config().temperature) || undefined,
        max_tokens: Number(kernel.config().max_tokens) || undefined
      },
      signal: abortController.signal,
      onAssistantContent: () => {
        streamingStore.set(true);
      },
      onToolResult: async (toolName, content, isError) => {
        // persist a tool row so the loop keeps context and the UI shows it
        const toolMsg = {
          id: crypto.randomUUID(),
          convId: activeId,
          type: 'tool',
          role: 'tool',
          content: `${toolName}: ${content}`.slice(0, 2000),
          parent: null,
          children: [],
          timestamp: Date.now()
        };
        await persistMessage(toolMsg);
        messagesStore.update((msgs) => [...msgs, toolMsg]);
      }
    });
    assistant.content = result.content ?? '';
    assistant.reasoning = result.reasoning ?? '';
    assistant.toolCalls = result.toolCalls ?? [];
    streamContentStore.set(assistant.content);
    messagesStore.update((msgs) => msgs.map((m) => (m.id === assistant.id ? { ...assistant } : m)));
    await db.upsertMessage({
      ...assistant,
      content: assistant.content,
      reasoning: assistant.reasoning,
      toolCalls: assistant.toolCalls
    });
    await db.updateConversation(activeId, { lastModified: Date.now() });
    loadConversations();
    refreshContext();
  } catch (err) {
    const msg = err?.message ?? String(err);
    if (err?.code) log.error(err.code, msg, err.detail);
    else log.error('LLMUI-STR-004', 'chat: stream failed', msg);
    const { toast } = await import('../../kernel/index.js');
    toast(t('Request failed') + ': ' + msg, 'error', 6000);
  } finally {
    streamingStore.set(false);
    streamContentStore.set('');
    abortController = null;
  }
}

export function abortStream() {
  abortController?.abort();
}

/** Poll /slots and publish the active slot's token usage for the gauge. */
export async function refreshContext() {
  if (!activeId) return;
  try {
    const { getSlots } = await import('../../kernel/api.js');
    const raw = await getSlots();
    const slots = Array.isArray(raw) ? raw : (raw?.data ?? []);
    const slot = [...slots].reverse().find((s) => s.n_past != null) ?? slots[0];
    if (slot?.n_past != null && slot?.n_ctx) {
      contextStore.set({ used: slot.n_past, total: slot.n_ctx });
    }
  } catch {
    // gauge stays hidden when slots are unavailable
  }
}

