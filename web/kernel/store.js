/**
 * store.js — tiny reactive store (pub/sub + snapshot).
 * Equivalent surface to the Svelte $state pattern used today:
 *   const s = store(initial);
 *   s.get() / s.set(v) / s.update(fn) / s.subscribe(fn) -> unsubscribe
 *   store.derived(fn, deps) — recompute when deps change
 */
export function store(initial) {
  let value = initial;
  const subs = new Set();
  return {
    get() {
      return value;
    },
    set(next) {
      value = next;
      for (const fn of subs) fn(value);
    },
    update(fn) {
      this.set(fn(value));
    },
    subscribe(fn) {
      subs.add(fn);
      fn(value);
      return () => subs.delete(fn);
    }
  };
}

/** Derived store: recompute when any dependency store changes. */
export function derived(fn, deps) {
  const s = store(fn());
  const refresh = () => s.set(fn());
  for (const d of deps) d.subscribe(refresh);
  return s;
}

/** Simple keyed pub/sub bus for cross-tree events. */
export function bus() {
  const handlers = new Map();
  return {
    on(event, fn) {
      if (!handlers.has(event)) handlers.set(event, new Set());
      handlers.get(event).add(fn);
      return () => handlers.get(event)?.delete(fn);
    },
    emit(event, payload) {
      handlers.get(event)?.forEach((fn) => fn(payload));
    }
  };
}
