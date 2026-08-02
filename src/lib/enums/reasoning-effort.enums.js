/**
 * Reasoning effort levels for thinking models.
 * These values are sent to the server and mapped to token budgets.
 */
export var ReasoningEffort;
(function (ReasoningEffort) {
    ReasoningEffort["DEFAULT"] = "default";
    ReasoningEffort["OFF"] = "off";
    ReasoningEffort["LOW"] = "low";
    ReasoningEffort["MEDIUM"] = "medium";
    ReasoningEffort["HIGH"] = "high";
    ReasoningEffort["MAX"] = "max";
})(ReasoningEffort || (ReasoningEffort = {}));
