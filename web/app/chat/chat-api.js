/**
 * chat-api.js — Agent A: public chat API used by render/composer/sidebar
 * (avoids circular imports between render.js and chat.js).
 */
import * as chat from './chat.js';

export const chatApi = {
  conversationsStore: chat.conversationsStore,
  activeConversationStore: chat.activeConversationStore,
  messagesStore: chat.messagesStore,
  streamingStore: chat.streamingStore,
  sendMessage: chat.sendMessage,
  abortStream: chat.abortStream,
  loadConversations: chat.loadConversations,
  openConversation: chat.openConversation,
  newConversation: chat.newConversation,
  deleteConversation: chat.deleteConversation,
  editMessage: chat.editMessage,
  deleteMessage: chat.deleteMessage,
  regenerateMessage: chat.regenerateMessage,
  renameConversation: chat.renameConversation,
  togglePin: chat.togglePin,
  applyPersona: chat.applyPersona,
  contextStore: chat.contextStore,
  forkConversation: chat.forkConversation,
  applyDefaultPersona: chat.applyDefaultPersona
};
