/**
 * sw.js — Agent B: hand-written service worker (Phase 2 PWA item).
 * Cache-first for hashed assets under /assets/* (immutable), network-first
 * for index.html and build.json. Lazy chunks are cached at runtime only —
 * never precached, so the SW never pins an unfinished feature.
 */
const CACHE = 'llama-ui-v1';

self.addEventListener('install', (event) => {
  event.waitUntil(caches.open(CACHE).then((c) => c.addAll(['/', '/manifest.webmanifest'])));
  self.skipWaiting();
});

self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches
      .keys()
      .then((keys) => Promise.all(keys.filter((k) => k !== CACHE).map((k) => caches.delete(k))))
  );
  self.clients.claim();
});

self.addEventListener('fetch', (event) => {
  const url = new URL(event.request.url);
  if (url.origin !== self.location.origin) return;

  // Hashed, immutable assets — cache first.
  if (url.pathname.startsWith('/assets/')) {
    event.respondWith(
      caches.match(event.request).then((hit) => {
        if (hit) return hit;
        return fetch(event.request).then((res) => {
          if (res.ok) {
            const clone = res.clone();
            caches.open(CACHE).then((c) => c.put(event.request, clone));
          }
          return res;
        });
      })
    );
    return;
  }

  // Navigation / index.html — network first, fall back to cache.
  if (event.request.mode === 'navigate') {
    event.respondWith(
      fetch(event.request).then((res) => {
        const clone = res.clone();
        caches.open(CACHE).then((c) => c.put('/', clone));
        return res;
      }).catch(() => caches.match('/').then((hit) => hit || caches.match(event.request)))
    );
    return;
  }

  // Everything else — stale-while-revalidate.
  event.respondWith(
    caches.match(event.request).then((hit) => {
      const fetched = fetch(event.request).then((res) => {
        if (res.ok) {
          const clone = res.clone();
          caches.open(CACHE).then((c) => c.put(event.request, clone));
        }
        return res;
      });
      return hit || fetched;
    })
  );
});
