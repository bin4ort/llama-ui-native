const WARNING_THRESHOLD = 80;
const CRITICAL_THRESHOLD = 95;
export function colorLevelFromPercent(percent) {
    if (percent === null)
        return 'neutral';
    if (percent >= CRITICAL_THRESHOLD)
        return 'critical';
    if (percent >= WARNING_THRESHOLD)
        return 'warning';
    return 'ok';
}
export function colorLevelTextClass(level) {
    switch (level) {
        case 'critical':
            return 'text-red-400';
        case 'warning':
            return 'text-amber-400';
        case 'ok':
            return 'text-muted-foreground';
        default:
            return 'text-muted-foreground';
    }
}
export function colorLevelBgClass(level) {
    switch (level) {
        case 'critical':
            return 'bg-red-500';
        case 'warning':
            return 'bg-amber-500';
        case 'ok':
            return 'bg-green-500';
        default:
            return 'bg-muted';
    }
}
