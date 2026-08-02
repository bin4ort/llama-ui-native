import { DEFAULT_CACHE_TTL_MS, DEFAULT_CACHE_MAX_ENTRIES } from '$lib/constants';
export class TTLCache {
    cache = new Map();
    ttlMs;
    maxEntries;
    onEvict;
    constructor(options = {}) {
        this.ttlMs = options.ttlMs ?? DEFAULT_CACHE_TTL_MS;
        this.maxEntries = options.maxEntries ?? DEFAULT_CACHE_MAX_ENTRIES;
        this.onEvict = options.onEvict;
    }
    /**
     * Get a value from cache. Returns null if expired or not found.
     */
    get(key) {
        const entry = this.cache.get(key);
        if (!entry)
            return null;
        if (Date.now() > entry.expiresAt) {
            this.delete(key);
            return null;
        }
        // Update last accessed time for LRU-like behavior
        entry.lastAccessed = Date.now();
        return entry.value;
    }
    /**
     * Set a value in cache with TTL.
     */
    set(key, value, customTtlMs) {
        // Evict oldest entries if at capacity
        if (this.cache.size >= this.maxEntries && !this.cache.has(key)) {
            this.evictOldest();
        }
        const ttl = customTtlMs ?? this.ttlMs;
        const now = Date.now();
        this.cache.set(key, {
            value,
            expiresAt: now + ttl,
            lastAccessed: now
        });
    }
    /**
     * Check if key exists and is not expired.
     */
    has(key) {
        const entry = this.cache.get(key);
        if (!entry)
            return false;
        if (Date.now() > entry.expiresAt) {
            this.delete(key);
            return false;
        }
        return true;
    }
    /**
     * Delete a specific key from cache.
     */
    delete(key) {
        const entry = this.cache.get(key);
        if (entry && this.onEvict) {
            this.onEvict(key, entry.value);
        }
        return this.cache.delete(key);
    }
    /**
     * Clear all entries from cache.
     */
    clear() {
        if (this.onEvict) {
            for (const [key, entry] of this.cache) {
                this.onEvict(key, entry.value);
            }
        }
        this.cache.clear();
    }
    /**
     * Get the number of entries (including potentially expired ones).
     */
    get size() {
        return this.cache.size;
    }
    /**
     * Remove all expired entries from cache.
     * Call periodically for proactive cleanup.
     */
    prune() {
        const now = Date.now();
        let pruned = 0;
        for (const [key, entry] of this.cache) {
            if (now > entry.expiresAt) {
                this.delete(key);
                pruned++;
            }
        }
        return pruned;
    }
    /**
     * Get all valid (non-expired) keys.
     */
    keys() {
        const now = Date.now();
        const validKeys = [];
        for (const [key, entry] of this.cache) {
            if (now <= entry.expiresAt) {
                validKeys.push(key);
            }
        }
        return validKeys;
    }
    /**
     * Evict the oldest (least recently accessed) entry.
     */
    evictOldest() {
        let oldestKey = null;
        let oldestTime = Infinity;
        for (const [key, entry] of this.cache) {
            if (entry.lastAccessed < oldestTime) {
                oldestTime = entry.lastAccessed;
                oldestKey = key;
            }
        }
        if (oldestKey !== null) {
            this.delete(oldestKey);
        }
    }
    /**
     * Refresh TTL for an existing entry without changing the value.
     */
    touch(key) {
        const entry = this.cache.get(key);
        if (!entry)
            return false;
        const now = Date.now();
        if (now > entry.expiresAt) {
            this.delete(key);
            return false;
        }
        entry.expiresAt = now + this.ttlMs;
        entry.lastAccessed = now;
        return true;
    }
}
/**
 * Reactive TTL Map for Svelte stores
 * Wraps SvelteMap with TTL functionality
 */
export class ReactiveTTLMap {
    entries = $state(new Map());
    ttlMs;
    maxEntries;
    constructor(options = {}) {
        this.ttlMs = options.ttlMs ?? DEFAULT_CACHE_TTL_MS;
        this.maxEntries = options.maxEntries ?? DEFAULT_CACHE_MAX_ENTRIES;
    }
    get(key) {
        const entry = this.entries.get(key);
        if (!entry)
            return null;
        if (Date.now() > entry.expiresAt) {
            this.entries.delete(key);
            return null;
        }
        entry.lastAccessed = Date.now();
        return entry.value;
    }
    set(key, value, customTtlMs) {
        if (this.entries.size >= this.maxEntries && !this.entries.has(key)) {
            this.evictOldest();
        }
        const ttl = customTtlMs ?? this.ttlMs;
        const now = Date.now();
        this.entries.set(key, {
            value,
            expiresAt: now + ttl,
            lastAccessed: now
        });
    }
    has(key) {
        const entry = this.entries.get(key);
        if (!entry)
            return false;
        if (Date.now() > entry.expiresAt) {
            this.entries.delete(key);
            return false;
        }
        return true;
    }
    delete(key) {
        return this.entries.delete(key);
    }
    clear() {
        this.entries.clear();
    }
    get size() {
        return this.entries.size;
    }
    prune() {
        const now = Date.now();
        let pruned = 0;
        for (const [key, entry] of this.entries) {
            if (now > entry.expiresAt) {
                this.entries.delete(key);
                pruned++;
            }
        }
        return pruned;
    }
    evictOldest() {
        let oldestKey = null;
        let oldestTime = Infinity;
        for (const [key, entry] of this.entries) {
            if (entry.lastAccessed < oldestTime) {
                oldestTime = entry.lastAccessed;
                oldestKey = key;
            }
        }
        if (oldestKey !== null) {
            this.entries.delete(oldestKey);
        }
    }
}
