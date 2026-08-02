/**
 * Creates a map of conversation IDs to their message counts from exported conversation data
 * @param exportedData - Array of exported conversations with their messages
 * @returns Map of conversation ID to message count
 */
export function createMessageCountMap(exportedData) {
    const countMap = new Map();
    for (const item of exportedData) {
        countMap.set(item.conv.id, item.messages.length);
    }
    return countMap;
}
/**
 * Gets the message count for a specific conversation from the count map
 * @param conversationId - The ID of the conversation
 * @param countMap - Map of conversation IDs to message counts
 * @returns The message count, or 0 if not found
 */
export function getMessageCount(conversationId, countMap) {
    return countMap.get(conversationId) ?? 0;
}
