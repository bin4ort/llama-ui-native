/**
 * Server role enum - used for single/multi-model mode
 */
export var ServerRole;
(function (ServerRole) {
    /** Single model mode - server running with a specific model loaded */
    ServerRole["MODEL"] = "model";
    /** Router mode - server managing multiple model instances */
    ServerRole["ROUTER"] = "router";
})(ServerRole || (ServerRole = {}));
/**
 * Model status enum - matches tools/server/server-models.h from C++ server
 * Used as the `value` field in the status object from /models endpoint
 */
export var ServerModelStatus;
(function (ServerModelStatus) {
    ServerModelStatus["UNLOADED"] = "unloaded";
    ServerModelStatus["LOADING"] = "loading";
    ServerModelStatus["LOADED"] = "loaded";
    ServerModelStatus["SLEEPING"] = "sleeping";
    ServerModelStatus["FAILED"] = "failed";
})(ServerModelStatus || (ServerModelStatus = {}));
/**
 * /models/sse event type enum - discriminates the records broadcast on the
 * model status feed in ROUTER mode. Matches the event names emitted by
 * tools/server/server-models.cpp from the C++ server.
 */
export var ServerModelsSseEventType;
(function (ServerModelsSseEventType) {
    ServerModelsSseEventType["STATUS_CHANGE"] = "status_change";
    ServerModelsSseEventType["MODEL_STATUS"] = "model_status";
    ServerModelsSseEventType["STATUS_UPDATE"] = "status_update";
    ServerModelsSseEventType["MODELS_RELOAD"] = "models_reload";
    ServerModelsSseEventType["MODEL_REMOVE"] = "model_remove";
    ServerModelsSseEventType["DOWNLOAD_PROGRESS"] = "download_progress";
})(ServerModelsSseEventType || (ServerModelsSseEventType = {}));
