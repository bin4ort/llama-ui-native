<script>import { extractSearchQuery, extractSearchResults, isWebSearchToolName } from '$lib/utils';
let { section, attachments, open, isStreaming, isExecuting, onToggle } = $props();
const searchResults = $derived(extractSearchResults(section.toolResult));
const searchQuery = $derived(extractSearchQuery(section.toolArgs));
const isSearchCall = $derived(searchResults.length > 0 || (searchQuery.length > 0 && isWebSearchToolName(section.toolName)));
</script>

{#if isSearchCall}
	<ChatMessageToolCallBlockSearchResults {section} {open} {isStreaming} {onToggle} />
{:else if section.toolName === BuiltInTool.GET_DATETIME}
	<ChatMessageToolCallBlockGetDatetime {section} {isStreaming} />
{:else if section.toolName === BuiltInTool.READ_FILE}
	<ChatMessageToolCallBlockReadFile {section} {open} {isStreaming} {onToggle} />
{:else if section.toolName === BuiltInTool.EDIT_FILE}
	<ChatMessageToolCallBlockEditFile {section} {open} {isStreaming} {onToggle} />
{:else if section.toolName === BuiltInTool.WRITE_FILE}
	<ChatMessageToolCallBlockWriteFile {section} {open} {isStreaming} {onToggle} />
{:else if section.toolName === BuiltInTool.EXEC_SHELL_COMMAND}
	<ChatMessageToolCallBlockExecShellCommand
		{section}
		{open}
		{isStreaming}
		{isExecuting}
		{attachments}
		{onToggle}
	/>
{:else if section.toolName === BuiltInTool.FILE_GLOB_SEARCH}
	<ChatMessageToolCallBlockFileGlobSearch {section} {open} {isStreaming} {onToggle} />
{:else if section.toolName === BuiltInTool.GREP_SEARCH}
	<ChatMessageToolCallBlockGrepSearch {section} {open} {isStreaming} {onToggle} />
{:else if section.toolName === BuiltInTool.RUN_JAVASCRIPT}
	<ChatMessageToolCallBlockRunJavascript {section} {open} {isStreaming} {onToggle} />
{:else}
	<ChatMessageToolCallBlockDefault {section} {open} {isStreaming} {attachments} {onToggle} />
{/if}
