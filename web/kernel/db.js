/**
 * db.js — Dexie wrapper. Schema identical to the current app:
 *   LlamaUi v1: conversations(id, lastModified, currNode, name),
 *              messages(id, convId, type, role, timestamp, parent, children)
 */
import Dexie from 'dexie';

export const db = new Dexie('LlamaUi');

db.version(1).stores({
  conversations: 'id, lastModified, currNode, name',
  messages: 'id, convId, type, role, timestamp, parent, children'
});

export async function getConversation(id) {
  return db.conversations.get(id);
}

export async function listConversations() {
  return db.conversations.toArray();
}

export async function getMessagesByConversation(convId) {
  return db.messages.where('convId').equals(convId).toArray();
}

export async function addConversation(conversation) {
  return db.conversations.add(conversation);
}

export async function updateConversation(id, patch) {
  return db.conversations.update(id, patch);
}

export async function deleteConversation(id) {
  await db.transaction('rw', db.conversations, db.messages, async () => {
    await db.messages.where('convId').equals(id).delete();
    await db.conversations.delete(id);
  });
}

export async function addMessage(message) {
  return db.messages.add(message);
}

export async function updateMessage(id, patch) {
  return db.messages.update(id, patch);
}

export async function deleteMessage(id) {
  return db.messages.delete(id);
}
