export function parseExecShellCommandError(toolResultString) {
    if (!toolResultString)
        return undefined;
    try {
        const parsed = JSON.parse(toolResultString);
        if (parsed &&
            typeof parsed === 'object' &&
            !Array.isArray(parsed) &&
            typeof parsed.error === 'string') {
            return parsed.error;
        }
    }
    catch {
        // Plain-text result = stdout/stderr, no structured error to surface.
    }
    return undefined;
}
