/**
 * Parameter source - indicates whether a parameter uses default or custom value
 */
export var ParameterSource;
(function (ParameterSource) {
    ParameterSource["DEFAULT"] = "default";
    ParameterSource["CUSTOM"] = "custom";
})(ParameterSource || (ParameterSource = {}));
/**
 * Syncable parameter type - data types for parameters that can be synced with server
 */
export var SyncableParameterType;
(function (SyncableParameterType) {
    SyncableParameterType["NUMBER"] = "number";
    SyncableParameterType["STRING"] = "string";
    SyncableParameterType["BOOLEAN"] = "boolean";
})(SyncableParameterType || (SyncableParameterType = {}));
/**
 * Settings field type - defines the input type for settings fields
 */
export var SettingsFieldType;
(function (SettingsFieldType) {
    SettingsFieldType["INPUT"] = "input";
    SettingsFieldType["TEXTAREA"] = "textarea";
    SettingsFieldType["CHECKBOX"] = "checkbox";
    SettingsFieldType["SELECT"] = "select";
    SettingsFieldType["RADIO"] = "radio";
})(SettingsFieldType || (SettingsFieldType = {}));
