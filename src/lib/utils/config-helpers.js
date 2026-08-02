/**
 * Type-safe configuration helpers
 *
 * Provides utilities for safely accessing and modifying configuration objects
 * with dynamic keys while maintaining TypeScript type safety.
 */
/**
 * Type-safe helper to access config properties dynamically
 * Provides better type safety than direct casting to Record
 */
export function setConfigValue(config, key, value) {
    if (key in config) {
        config[key] = value;
    }
}
/**
 * Type-safe helper to get config values dynamically
 */
export function getConfigValue(config, key) {
    const value = config[key];
    return value;
}
/**
 * Convert a SettingsConfigType to a ParameterRecord for specific keys
 * Useful for parameter synchronization operations
 */
export function configToParameterRecord(config, keys) {
    const record = {};
    for (const key of keys) {
        const value = getConfigValue(config, key);
        if (value !== undefined) {
            record[key] = value;
        }
    }
    return record;
}
