import { NEW_CHAT_DRAFT_KEY } from '$lib/constants';
class DraftMessagesStore {
    drafts = new Map();
    getDraftMessage(chatId) {
        const key = chatId ?? NEW_CHAT_DRAFT_KEY;
        return this.drafts.get(key) ?? { message: '', files: [] };
    }
    saveDraftMessage(chatId, message, files) {
        const key = chatId ?? NEW_CHAT_DRAFT_KEY;
        if (message || files.length > 0) {
            this.drafts.set(key, { message, files: [...files] });
        }
        else {
            this.drafts.delete(key);
        }
    }
    clearDraftMessage(chatId) {
        const key = chatId ?? NEW_CHAT_DRAFT_KEY;
        this.drafts.delete(key);
    }
}
export const draftMessagesStore = new DraftMessagesStore();
