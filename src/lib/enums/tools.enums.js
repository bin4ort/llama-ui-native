export var ToolSource;
(function (ToolSource) {
    ToolSource["BUILTIN"] = "builtin";
    ToolSource["MCP"] = "mcp";
    ToolSource["CUSTOM"] = "custom";
    ToolSource["FRONTEND"] = "frontend";
})(ToolSource || (ToolSource = {}));
export var ToolPermissionDecision;
(function (ToolPermissionDecision) {
    ToolPermissionDecision["ALWAYS"] = "always";
    ToolPermissionDecision["ALWAYS_SERVER"] = "always_server";
    ToolPermissionDecision["ONCE"] = "once";
    ToolPermissionDecision["DENY"] = "deny";
})(ToolPermissionDecision || (ToolPermissionDecision = {}));
export var ToolResponseField;
(function (ToolResponseField) {
    ToolResponseField["PLAIN_TEXT"] = "plain_text_response";
    ToolResponseField["ERROR"] = "error";
})(ToolResponseField || (ToolResponseField = {}));
/**
 * Wire-format identifiers for built-in and frontend tools. The string
 * value matches what the model emits in tool call names, so comparing
 * against `BuiltInTool.READ_FILE` is equivalent to comparing against the
 * raw `'read_file'` literal - the enum just keeps the two in lock-step
 * and gives TypeScript a single source of truth for autocomplete / rename
 * support.
 */
export var BuiltInTool;
(function (BuiltInTool) {
    BuiltInTool["READ_FILE"] = "read_file";
    BuiltInTool["EDIT_FILE"] = "edit_file";
    BuiltInTool["WRITE_FILE"] = "write_file";
    BuiltInTool["GET_DATETIME"] = "get_datetime";
    BuiltInTool["FILE_GLOB_SEARCH"] = "file_glob_search";
    BuiltInTool["GREP_SEARCH"] = "grep_search";
    BuiltInTool["EXEC_SHELL_COMMAND"] = "exec_shell_command";
    BuiltInTool["RUN_JAVASCRIPT"] = "run_javascript";
})(BuiltInTool || (BuiltInTool = {}));
