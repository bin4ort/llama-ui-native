import { ReasoningEffort } from '$lib/enums';
/**
 * Reasoning effort UI labels.
 * Keys match the ReasoningEffort enum values for type-safe lookups.
 */
export const REASONING_EFFORT_LABELS = {
    [ReasoningEffort.DEFAULT]: 'Default',
    [ReasoningEffort.OFF]: 'Off',
    [ReasoningEffort.LOW]: 'Low',
    [ReasoningEffort.MEDIUM]: 'Medium',
    [ReasoningEffort.HIGH]: 'High',
    [ReasoningEffort.MAX]: 'Max'
};
export const REASONING_EFFORT_LEVELS = [
    { value: ReasoningEffort.DEFAULT, label: 'Default' },
    { value: ReasoningEffort.OFF, label: 'Off' },
    { value: ReasoningEffort.LOW, label: 'Low' },
    { value: ReasoningEffort.MEDIUM, label: 'Medium' },
    { value: ReasoningEffort.HIGH, label: 'High' },
    { value: ReasoningEffort.MAX, label: 'Max', hasInfo: true }
];
