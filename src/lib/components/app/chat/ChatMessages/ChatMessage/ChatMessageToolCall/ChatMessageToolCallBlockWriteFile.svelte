<script>import { parseWriteFileMeta } from './parsers/write-file';
let { section, open, isStreaming, onToggle } = $props();
const writeFileMeta = $derived(parseWriteFileMeta(section));
</script>

<ToolCallBlock {section} {open} {isStreaming} meta={writeFileMeta} {onToggle}>
	{#snippet titleSnippet()}
		<span class="text-muted-foreground">{t('Write file')} </span>
		<span class="font-mono">{writeFileMeta?.filePath}</span>
		{#if writeFileMeta?.errorMessage}
			<span class="ml-1 text-xs italic text-muted-foreground/70">(failed)</span>
		{/if}
	{/snippet}

	{#snippet children(meta, ctx)}
		{#if meta?.errorMessage}
			<div
				class="flex items-start gap-2 rounded bg-red-500/10 p-2 text-xs text-red-600 italic dark:text-red-400"
			>
				<XCircle class="mt-0.5 h-3 w-3 shrink-0" />
				<span>{meta.errorMessage}</span>
			</div>
		{:else if meta}
			<SyntaxHighlightedCode
				code={meta.content}
				language={meta.language}
				maxHeight={MAX_HEIGHT_CODE_BLOCK}
				streaming={ctx.isCodeStreaming}
			/>
			<div class="mt-1.5 text-xs text-muted-foreground/70 italic">
				{#if meta.resultMessage}
					{meta.resultMessage}{meta.bytesWritten != null ? RESULT_STAT_SEPARATOR : ''}{/if}
				{#if meta.bytesWritten != null}
					<span class="font-mono">{meta.bytesWritten}</span>
					bytes
				{/if}
			</div>
		{/if}
	{/snippet}
</ToolCallBlock>
