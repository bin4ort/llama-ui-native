/**
 * API base URL — reads serverEndpoint from localStorage directly,
 * bypassing Svelte stores to work in non-reactive utility modules.
 * When accessed from a remote device, rewrites localhost references
 * to use the page's actual hostname.
 */
const LOCALSTORAGE_KEY = 'LlamaUi.config';
export function getApiBase() {
    try {
        const raw = localStorage.getItem(LOCALSTORAGE_KEY);
        if (raw) {
            const config = JSON.parse(raw);
            const ep = config.serverEndpoint;
            if (ep && typeof ep === 'string' && ep.trim() !== '') {
                const trimmed = ep.trim().replace(/\/+$/, '');
                // If endpoint points to localhost but page is accessed remotely,
                // use the page's hostname so API calls reach the same machine
                if (/\/\/(localhost|127\.0\.0\.1)/.test(trimmed) &&
                    window.location.hostname !== 'localhost' &&
                    window.location.hostname !== '127.0.0.1') {
                    return trimmed.replace(/\/\/(localhost|127\.0\.0\.1)/, '//' + window.location.hostname);
                }
                return trimmed;
            }
        }
    }
    catch {
        // localStorage not available or JSON parse error
    }
    return `http://${window.location.hostname}:8080`;
}
