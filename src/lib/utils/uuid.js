export function uuid() {
    return globalThis.crypto?.randomUUID?.() ?? Math.random().toString(36).substring(2);
}
