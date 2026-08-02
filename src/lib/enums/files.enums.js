/**
 * Comprehensive dictionary of all supported file types in llama-ui
 * Organized by category with TypeScript enums for better type safety
 */
// File type category enum
export var FileTypeCategory;
(function (FileTypeCategory) {
    FileTypeCategory["IMAGE"] = "image";
    FileTypeCategory["AUDIO"] = "audio";
    FileTypeCategory["VIDEO"] = "video";
    FileTypeCategory["PDF"] = "pdf";
    FileTypeCategory["TEXT"] = "text";
})(FileTypeCategory || (FileTypeCategory = {}));
/**
 * Special file types for internal use (not MIME types)
 */
export var SpecialFileType;
(function (SpecialFileType) {
    SpecialFileType["MCP_PROMPT"] = "mcp-prompt";
})(SpecialFileType || (SpecialFileType = {}));
// Specific file type enums for each category
export var FileTypeImage;
(function (FileTypeImage) {
    FileTypeImage["JPEG"] = "jpeg";
    FileTypeImage["PNG"] = "png";
    FileTypeImage["GIF"] = "gif";
    FileTypeImage["WEBP"] = "webp";
    FileTypeImage["SVG"] = "svg";
    FileTypeImage["HEIC"] = "heic";
    FileTypeImage["HEIF"] = "heif";
})(FileTypeImage || (FileTypeImage = {}));
export var FileTypeAudio;
(function (FileTypeAudio) {
    FileTypeAudio["MP3"] = "mp3";
    FileTypeAudio["WAV"] = "wav";
    FileTypeAudio["WEBM"] = "webm";
})(FileTypeAudio || (FileTypeAudio = {}));
export var FileTypeVideo;
(function (FileTypeVideo) {
    FileTypeVideo["MP4"] = "mp4";
    FileTypeVideo["OGG"] = "ogg";
})(FileTypeVideo || (FileTypeVideo = {}));
export var FileTypePdf;
(function (FileTypePdf) {
    FileTypePdf["PDF"] = "pdf";
})(FileTypePdf || (FileTypePdf = {}));
export var FileTypeText;
(function (FileTypeText) {
    FileTypeText["PLAIN_TEXT"] = "plainText";
    FileTypeText["MARKDOWN"] = "md";
    FileTypeText["ASCIIDOC"] = "asciidoc";
    FileTypeText["JAVASCRIPT"] = "js";
    FileTypeText["TYPESCRIPT"] = "ts";
    FileTypeText["JSX"] = "jsx";
    FileTypeText["TSX"] = "tsx";
    FileTypeText["CSS"] = "css";
    FileTypeText["HTML"] = "html";
    FileTypeText["JSON"] = "json";
    FileTypeText["XML"] = "xml";
    FileTypeText["YAML"] = "yaml";
    FileTypeText["CSV"] = "csv";
    FileTypeText["LOG"] = "log";
    FileTypeText["PYTHON"] = "python";
    FileTypeText["JAVA"] = "java";
    FileTypeText["CPP"] = "cpp";
    FileTypeText["PHP"] = "php";
    FileTypeText["RUBY"] = "ruby";
    FileTypeText["GO"] = "go";
    FileTypeText["RUST"] = "rust";
    FileTypeText["SHELL"] = "shell";
    FileTypeText["SQL"] = "sql";
    FileTypeText["R"] = "r";
    FileTypeText["SCALA"] = "scala";
    FileTypeText["KOTLIN"] = "kotlin";
    FileTypeText["SWIFT"] = "swift";
    FileTypeText["DART"] = "dart";
    FileTypeText["VUE"] = "vue";
    FileTypeText["SVELTE"] = "svelte";
    FileTypeText["LATEX"] = "latex";
    FileTypeText["BIBTEX"] = "bibtex";
    FileTypeText["CUDA"] = "cuda";
    FileTypeText["VULKAN"] = "vulkan";
    FileTypeText["HASKELL"] = "haskell";
    FileTypeText["CSHARP"] = "csharp";
    FileTypeText["PROPERTIES"] = "properties";
})(FileTypeText || (FileTypeText = {}));
// File extension enums
export var FileExtensionImage;
(function (FileExtensionImage) {
    FileExtensionImage["JPG"] = ".jpg";
    FileExtensionImage["JPEG"] = ".jpeg";
    FileExtensionImage["PNG"] = ".png";
    FileExtensionImage["GIF"] = ".gif";
    FileExtensionImage["WEBP"] = ".webp";
    FileExtensionImage["SVG"] = ".svg";
    FileExtensionImage["HEIC"] = ".heic";
    FileExtensionImage["HEIF"] = ".heif";
})(FileExtensionImage || (FileExtensionImage = {}));
export var FileExtensionAudio;
(function (FileExtensionAudio) {
    FileExtensionAudio["MP3"] = ".mp3";
    FileExtensionAudio["WAV"] = ".wav";
})(FileExtensionAudio || (FileExtensionAudio = {}));
export var FileExtensionVideo;
(function (FileExtensionVideo) {
    FileExtensionVideo["MP4"] = ".mp4";
    FileExtensionVideo["OGG"] = ".ogg";
})(FileExtensionVideo || (FileExtensionVideo = {}));
export var FileExtensionPdf;
(function (FileExtensionPdf) {
    FileExtensionPdf["PDF"] = ".pdf";
})(FileExtensionPdf || (FileExtensionPdf = {}));
export var FileExtensionText;
(function (FileExtensionText) {
    FileExtensionText["TXT"] = ".txt";
    FileExtensionText["MD"] = ".md";
    FileExtensionText["ADOC"] = ".adoc";
    FileExtensionText["JS"] = ".js";
    FileExtensionText["TS"] = ".ts";
    FileExtensionText["JSX"] = ".jsx";
    FileExtensionText["TSX"] = ".tsx";
    FileExtensionText["CSS"] = ".css";
    FileExtensionText["HTML"] = ".html";
    FileExtensionText["HTM"] = ".htm";
    FileExtensionText["JSON"] = ".json";
    FileExtensionText["JSONL"] = ".jsonl";
    FileExtensionText["ZIP"] = ".zip";
    FileExtensionText["XML"] = ".xml";
    FileExtensionText["YAML"] = ".yaml";
    FileExtensionText["YML"] = ".yml";
    FileExtensionText["CSV"] = ".csv";
    FileExtensionText["LOG"] = ".log";
    FileExtensionText["PY"] = ".py";
    FileExtensionText["JAVA"] = ".java";
    FileExtensionText["CPP"] = ".cpp";
    FileExtensionText["C"] = ".c";
    FileExtensionText["H"] = ".h";
    FileExtensionText["PHP"] = ".php";
    FileExtensionText["RB"] = ".rb";
    FileExtensionText["GO"] = ".go";
    FileExtensionText["RS"] = ".rs";
    FileExtensionText["SH"] = ".sh";
    FileExtensionText["BAT"] = ".bat";
    FileExtensionText["SQL"] = ".sql";
    FileExtensionText["R"] = ".r";
    FileExtensionText["SCALA"] = ".scala";
    FileExtensionText["KT"] = ".kt";
    FileExtensionText["SWIFT"] = ".swift";
    FileExtensionText["DART"] = ".dart";
    FileExtensionText["VUE"] = ".vue";
    FileExtensionText["SVELTE"] = ".svelte";
    FileExtensionText["TEX"] = ".tex";
    FileExtensionText["BIB"] = ".bib";
    FileExtensionText["CU"] = ".cu";
    FileExtensionText["CUH"] = ".cuh";
    FileExtensionText["COMP"] = ".comp";
    FileExtensionText["HPP"] = ".hpp";
    FileExtensionText["HS"] = ".hs";
    FileExtensionText["PROPERTIES"] = ".properties";
    FileExtensionText["CS"] = ".cs";
})(FileExtensionText || (FileExtensionText = {}));
// MIME type prefixes and includes for content detection
export var MimeTypePrefix;
(function (MimeTypePrefix) {
    MimeTypePrefix["IMAGE"] = "image/";
    MimeTypePrefix["TEXT"] = "text";
})(MimeTypePrefix || (MimeTypePrefix = {}));
export var MimeTypeIncludes;
(function (MimeTypeIncludes) {
    MimeTypeIncludes["JSON"] = "json";
    MimeTypeIncludes["JAVASCRIPT"] = "javascript";
    MimeTypeIncludes["TYPESCRIPT"] = "typescript";
})(MimeTypeIncludes || (MimeTypeIncludes = {}));
// URI patterns for content detection
export var UriPattern;
(function (UriPattern) {
    UriPattern["DATABASE_KEYWORD"] = "database";
    UriPattern["DATABASE_SCHEME"] = "db://";
})(UriPattern || (UriPattern = {}));
// MIME type enums
export var MimeTypeApplication;
(function (MimeTypeApplication) {
    MimeTypeApplication["JSON"] = "application/json";
    MimeTypeApplication["PDF"] = "application/pdf";
    MimeTypeApplication["OCTET_STREAM"] = "application/octet-stream";
    MimeTypeApplication["ZIP"] = "application/zip";
})(MimeTypeApplication || (MimeTypeApplication = {}));
export var MimeTypeAudio;
(function (MimeTypeAudio) {
    MimeTypeAudio["MP3_MPEG"] = "audio/mpeg";
    MimeTypeAudio["MP3"] = "audio/mp3";
    MimeTypeAudio["MP4"] = "audio/mp4";
    MimeTypeAudio["WAV"] = "audio/wav";
    MimeTypeAudio["WAVE"] = "audio/wave";
    MimeTypeAudio["X_WAV"] = "audio/x-wav";
    MimeTypeAudio["X_WAVE"] = "audio/x-wave";
    MimeTypeAudio["VND_WAVE"] = "audio/vnd.wave";
    MimeTypeAudio["X_PN_WAV"] = "audio/x-pn-wav";
    MimeTypeAudio["WEBM"] = "audio/webm";
    MimeTypeAudio["WEBM_OPUS"] = "audio/webm;codecs=opus";
})(MimeTypeAudio || (MimeTypeAudio = {}));
export var MimeTypeVideo;
(function (MimeTypeVideo) {
    MimeTypeVideo["MP4"] = "video/mp4";
    MimeTypeVideo["OGG"] = "video/ogg";
})(MimeTypeVideo || (MimeTypeVideo = {}));
export var MimeTypeImage;
(function (MimeTypeImage) {
    MimeTypeImage["JPEG"] = "image/jpeg";
    MimeTypeImage["JPG"] = "image/jpg";
    MimeTypeImage["PNG"] = "image/png";
    MimeTypeImage["GIF"] = "image/gif";
    MimeTypeImage["WEBP"] = "image/webp";
    MimeTypeImage["SVG"] = "image/svg+xml";
    MimeTypeImage["ICO"] = "image/x-icon";
    MimeTypeImage["ICO_MICROSOFT"] = "image/vnd.microsoft.icon";
    MimeTypeImage["HEIC"] = "image/heic";
    MimeTypeImage["HEIF"] = "image/heif";
})(MimeTypeImage || (MimeTypeImage = {}));
export var MimeTypeText;
(function (MimeTypeText) {
    MimeTypeText["PLAIN"] = "text/plain";
    MimeTypeText["MARKDOWN"] = "text/markdown";
    MimeTypeText["ASCIIDOC"] = "text/asciidoc";
    MimeTypeText["JAVASCRIPT"] = "text/javascript";
    MimeTypeText["JAVASCRIPT_APP"] = "application/javascript";
    MimeTypeText["TYPESCRIPT"] = "text/typescript";
    MimeTypeText["JSX"] = "text/jsx";
    MimeTypeText["TSX"] = "text/tsx";
    MimeTypeText["CSS"] = "text/css";
    MimeTypeText["HTML"] = "text/html";
    MimeTypeText["JSON"] = "application/json";
    MimeTypeText["JSONL"] = "application/jsonl";
    MimeTypeText["XML_TEXT"] = "text/xml";
    MimeTypeText["XML_APP"] = "application/xml";
    MimeTypeText["YAML_TEXT"] = "text/yaml";
    MimeTypeText["YAML_APP"] = "application/yaml";
    MimeTypeText["CSV"] = "text/csv";
    MimeTypeText["PYTHON"] = "text/x-python";
    MimeTypeText["JAVA"] = "text/x-java-source";
    MimeTypeText["CPP_HDR"] = "text/x-c++hdr";
    MimeTypeText["CPP_SRC"] = "text/x-c++src";
    MimeTypeText["CSHARP"] = "text/x-csharp";
    MimeTypeText["HASKELL"] = "text/x-haskell";
    MimeTypeText["C_SRC"] = "text/x-csrc";
    MimeTypeText["C_HDR"] = "text/x-chdr";
    MimeTypeText["PHP"] = "text/x-php";
    MimeTypeText["RUBY"] = "text/x-ruby";
    MimeTypeText["GO"] = "text/x-go";
    MimeTypeText["RUST"] = "text/x-rust";
    MimeTypeText["SHELL"] = "text/x-shellscript";
    MimeTypeText["BAT"] = "application/x-bat";
    MimeTypeText["SQL"] = "text/x-sql";
    MimeTypeText["R"] = "text/x-r";
    MimeTypeText["SCALA"] = "text/x-scala";
    MimeTypeText["KOTLIN"] = "text/x-kotlin";
    MimeTypeText["SWIFT"] = "text/x-swift";
    MimeTypeText["DART"] = "text/x-dart";
    MimeTypeText["VUE"] = "text/x-vue";
    MimeTypeText["SVELTE"] = "text/x-svelte";
    MimeTypeText["TEX"] = "text/x-tex";
    MimeTypeText["TEX_APP"] = "application/x-tex";
    MimeTypeText["LATEX"] = "application/x-latex";
    MimeTypeText["BIBTEX"] = "text/x-bibtex";
    MimeTypeText["CUDA"] = "text/x-cuda";
    MimeTypeText["PROPERTIES"] = "text/properties";
})(MimeTypeText || (MimeTypeText = {}));
