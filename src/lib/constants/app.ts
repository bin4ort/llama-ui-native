export const APP_NAME = import.meta.env?.VITE_PUBLIC_APP_NAME || 'llama-ui';

// Single source of truth for the app version. Keep in sync with:
//   - package.json ("version")
//   - server.h (VERSION / BUILD)
// Bump APP_BUILD (hex) on every release, not every commit — git is the history.
export const APP_VERSION = '0.4.3';
export const APP_BUILD = '0x07D21';
