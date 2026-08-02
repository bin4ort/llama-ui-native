/**
 * Connection lifecycle phases for MCP protocol
 */
export var MCPConnectionPhase;
(function (MCPConnectionPhase) {
    MCPConnectionPhase["IDLE"] = "idle";
    MCPConnectionPhase["TRANSPORT_CREATING"] = "transport_creating";
    MCPConnectionPhase["TRANSPORT_READY"] = "transport_ready";
    MCPConnectionPhase["INITIALIZING"] = "initializing";
    MCPConnectionPhase["CAPABILITIES_EXCHANGED"] = "capabilities_exchanged";
    MCPConnectionPhase["LISTING_TOOLS"] = "listing_tools";
    MCPConnectionPhase["CONNECTED"] = "connected";
    MCPConnectionPhase["ERROR"] = "error";
    MCPConnectionPhase["DISCONNECTED"] = "disconnected";
})(MCPConnectionPhase || (MCPConnectionPhase = {}));
/**
 * Log level for connection events
 */
export var MCPLogLevel;
(function (MCPLogLevel) {
    MCPLogLevel["INFO"] = "info";
    MCPLogLevel["WARN"] = "warn";
    MCPLogLevel["ERROR"] = "error";
})(MCPLogLevel || (MCPLogLevel = {}));
/**
 * Transport types for MCP connections
 */
export var MCPTransportType;
(function (MCPTransportType) {
    MCPTransportType["WEBSOCKET"] = "websocket";
    MCPTransportType["STREAMABLE_HTTP"] = "streamable_http";
    MCPTransportType["SSE"] = "sse";
})(MCPTransportType || (MCPTransportType = {}));
/**
 * Health check status for MCP servers
 */
export var HealthCheckStatus;
(function (HealthCheckStatus) {
    HealthCheckStatus["IDLE"] = "idle";
    HealthCheckStatus["CONNECTING"] = "connecting";
    HealthCheckStatus["SUCCESS"] = "success";
    HealthCheckStatus["ERROR"] = "error";
})(HealthCheckStatus || (HealthCheckStatus = {}));
/**
 * Content types for MCP tool results
 */
export var MCPContentType;
(function (MCPContentType) {
    MCPContentType["TEXT"] = "text";
    MCPContentType["IMAGE"] = "image";
    MCPContentType["RESOURCE"] = "resource";
})(MCPContentType || (MCPContentType = {}));
/**
 * JSON Schema types used in MCP tool definitions
 */
export var JsonSchemaType;
(function (JsonSchemaType) {
    JsonSchemaType["OBJECT"] = "object";
    JsonSchemaType["STRING"] = "string";
    JsonSchemaType["NUMBER"] = "number";
})(JsonSchemaType || (JsonSchemaType = {}));
/**
 * Reference types for MCP completions
 */
export var MCPRefType;
(function (MCPRefType) {
    MCPRefType["PROMPT"] = "ref/prompt";
    MCPRefType["RESOURCE"] = "ref/resource";
})(MCPRefType || (MCPRefType = {}));
