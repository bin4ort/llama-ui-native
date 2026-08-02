import { browser } from '$app/environment';
export function persisted(key, initialValue) {
    let value = initialValue;
    if (browser) {
        try {
            const stored = localStorage.getItem(key);
            if (stored !== null) {
                value = JSON.parse(stored);
            }
        }
        catch (error) {
            console.warn(`Failed to load ${key}:`, error);
        }
    }
    const persist = (next) => {
        if (!browser) {
            return;
        }
        try {
            if (next === null || next === undefined) {
                localStorage.removeItem(key);
                return;
            }
            localStorage.setItem(key, JSON.stringify(next));
        }
        catch (error) {
            console.warn(`Failed to persist ${key}:`, error);
        }
    };
    return {
        get value() {
            return value;
        },
        set value(newValue) {
            value = newValue;
            persist(newValue);
        }
    };
}
