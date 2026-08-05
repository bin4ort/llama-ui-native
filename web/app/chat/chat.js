/**
 * chat.js — Agent A: chat state (active conversation, message tree, send +
 * SSE streaming, reasoning/tool-call extraction) over the frozen kernel.
 * Error codes used (never edited): LLMUI-API-*, LLMUI-STR-*, LLMUI-CHAT-*,
 * LLMUI-TL-*.
 */
import * as kernel from '../../kernel/index.js';
import { streamChatCompletion } from '../../kernel/api.js';

const { store, db, log, presets } = kernel;

export const conversationsStore = store([]); // flat conversations (sorted by lastModified desc)
export const activeConversationStore = store(null);
export const messagesStore = store([]); // flat messages of the active conversation
export const streamingStore = store(false);
export const streamContentStore = store('');

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

function buildUserMessage(convId, content) {
  return {
    id: crypto.randomUUID(),
    convId,
    type: 'user',
    role: 'user',
    content,
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
    } else if (m.role === 'user' || m.role === 'assistant') {
      out.push({ role: m.role, content: m.content });
    }
  }
  return out;
}

/**
 * Send a user message: persist, then stream the completion. Appends the
 * assistant message with delta content, reasoning and tool-call fields.
 */
export async function sendMessage(content) {
  if (!activeId || !content.trim()) return;
  abortController = new AbortController();

  const userMsg = buildUserMessage(activeId, content.trim());
  await persistMessage(userMsg);
  messagesStore.update((msgs) => [...msgs, userMsg]);

  const assistant = buildAssistantMessage(activeId);
  messagesStore.update((msgs) => [...msgs, assistant]);
  streamingStore.set(true);
  streamContentStore.set('');

  const apiMessages = toApiMessages(messagesStore.get().filter((m) => m.id !== assistant.id));

  try {
    await streamChatCompletion({
      messages: apiMessages,
      options: {
        temperature: Number(kernel.config().temperature) || undefined,
        max_tokens: Number(kernel.config().max_tokens) || undefined
      },
      signal: abortController.signal,
      onData: (json) => {
        const delta = json.choices?.[0]?.delta ?? {};
        const content = delta.content ?? '';
        const reasoning = delta.reasoning_content ?? '';
        if (content || reasoning) {
          assistant.content += content;
          if (reasoning) {
            assistant.reasoning ??= '';
            assistant.reasoning += reasoning;
          }
          streamContentStore.set(assistant.content);
          messagesStore.update((msgs) => msgs.map((m) => (m.id === assistant.id ? { ...assistant } : m)));
        }
        if (delta.tool_calls?.length) {
          assistant.toolCalls = mergeToolCalls(assistant.toolCalls ?? [], delta.tool_calls);
        }
      },
      onDone: async () => {
        await db.updateMessage(assistant.id, {
          content: assistant.content,
          reasoning: assistant.reasoning ?? '',
          toolCalls: assistant.toolCalls ?? []
        });
        await db.updateConversation(activeId, { lastModified: Date.now() });
        loadConversations();
      }
    });
  } catch (err) {
    if (err?.code) log.error(err.code, err.message, err.detail);
    else log.error('LLMUI-STR-004', 'chat: stream failed', String(err));
  } finally {
    streamingStore.set(false);
    streamContentStore.set('');
    abortController = null;
  }
}

export function abortStream() {
  abortController?.abort();
}

function mergeToolCalls(existing, incoming) {
  const map = new Map(existing.map((tc) => [tc.index, tc]));
  for (const tc of incoming) {
    const current = map.get(tc.index) ?? { index: tc.index, id: '', type: 'function', function: { name: '', arguments: '' } };
    current.id = tc.id || current.id;
    current.function.name += tc.function?.name ?? '';
    current.function.arguments += tc.function?.arguments ?? '';
    map.set(tc.index, current);
  }
  return [...map.values()];
}
