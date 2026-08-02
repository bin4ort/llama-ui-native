<script>import { parseReadFileMeta } from './parsers/read-file';
let { section, open, isStreaming, onToggle } = $props();
const readFileMeta = $derived(parseReadFileMeta(section));
</script>

<ToolCallBlock {section} {open} {isStreaming} meta={readFileMeta} {onToggle}>
	{#snippet titleSnippet()}
		<span class="text-muted-foreground">{t('Read file')} </span>
		<span class="font-mono">{readFileMeta?.fileName}</span>
		{#if readFileMeta?.lineRange}
			<span class="text-muted-foreground"
				>&nbsp;(lines {readFileMeta.lineRange.start}-{readFileMeta.lineRange.end})</span
			>
		{/if}
	{/snippet}

	{#snippet children(_meta, _ctx)}
		{#if section.toolResult}
			<SyntaxHighlightedCode
				code={section.toolResult}
				language={readFileMeta?.language ?? DEFAULT_LANGUAGE}
				maxHeight={MAX_HEIGHT_CODE_BLOCK}
			/>
		{:else}
			<div class="rounded bg-muted/20 p-2 text-xs text-muted-foreground/70 italic">
				{t("Waiting for file content...")}
			</div>
		{/if}
	{/snippet}
</ToolCallBlock>
