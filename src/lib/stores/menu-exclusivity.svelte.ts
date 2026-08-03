/**
 * Menu exclusivity — at most one chat-bar dropdown can be open at a time.
 * Each menu registers a close function under a stable key; opening a menu
 * closes every other registered menu.
 */

const closeFns = new Map<string, () => void>();

export function registerMenuClose(key: string, close: () => void): () => void {
	closeFns.set(key, close);
	return () => {
		if (closeFns.get(key) === close) closeFns.delete(key);
	};
}

export function closeOtherMenus(key: string): void {
	for (const [otherKey, close] of closeFns) {
		if (otherKey !== key) close();
	}
}
