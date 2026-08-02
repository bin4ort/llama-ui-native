<script>import { classifyToolResult, parseToolResultWithImages } from '$lib/utils';
import { getBuiltinToolUi } from '$lib/constants/built-in-tools';
let { section, open, isStreaming, attachments, onToggle } = $props();
const title = $derived(getBuiltinToolUi(section.toolName)?.label ?? section.toolName ?? '');
const parsedLines = $derived(section.toolResult ? parseToolResultWithImages(section.toolResult, attachments) : []);
const outputKind = $derived(classifyToolResult(section.toolResult));
</script>

<ToolCallBlock {section} {open} {isStreaming} meta={null} {title} {onToggle}>
	{#snippet children(_meta, ctx)}
		{#if ctx.isStreamingCall}
			<div class="mb-2 flex items-center gap-2 text-xs text-muted-foreground/70">
				<span>{t('Input')}</span>
				{#if ctx.isStreaming}
					<Loader2 class="h-3 w-3 animate-spin" />
				{/if}
			</div>
			{#if section.toolArgs}
				<SyntaxHighlightedCode
					code={formatJsonPretty(section.toolArgs)}
					language={FileTypeText.JSON}
					maxHeight={MAX_HEIGHT_CODE_BLOCK}
					streaming={ctx.isCodeStreaming}
				/>
			{:else if ctx.isStreaming}
				<div class="rounded bg-muted/20 p-2 text-xs text-muted-foreground/70 italic">
					{t("Receiving arguments...")}
				</div>
			{:else}
				<div
					class="rounded bg-yellow-500/10 p-2 text-xs text-yellow-600 italic dark:text-yellow-400"
				>
					{t("Response was truncated")}
				</div>
			{/if}
		{:else}
			{@const showInput = Boolean(section.toolArgs)}
			{#if showInput}
				<div class="mb-1.5 flex items-center gap-2 text-xs text-muted-foreground/70">
					<span>Input</span>
				</div>
				<SyntaxHighlightedCode
					code={formatJsonPretty(section.toolArgs ?? '')}
					language={FileTypeText.JSON}
					maxHeight={MAX_HEIGHT_CODE_BLOCK}
					streaming={ctx.isCodeStreaming}
				/>
			{/if}
			<div
				class={showInput
					? 'mt-4 mb-1.5 flex items-center gap-2 text-xs text-muted-foreground/70'
					: 'mb-1.5 flex items-center gap-2 text-xs text-muted-foreground/70'}
			>
				<span>{t('Output')}</span>
				{#if ctx.isPending}
					<Loader2 class="h-3 w-3 animate-spin" />
				{/if}
			</div>
			{#if ctx.isPending}
				<div class="rounded bg-muted/20 p-2 text-xs text-muted-foreground/70 italic">
					{t("Waiting for result...")}
				</div>
			{:else if section.toolResult}
				{#if outputKind === ToolResultKind.JSON}
					<SyntaxHighlightedCode
						code={formatJsonPretty(section.toolResult)}
						language={FileTypeText.JSON}
						maxHeight={MAX_HEIGHT_CODE_BLOCK}
					/>
				{:else if outputKind === ToolResultKind.MARKDOWN}
					<MarkdownContent content={section.toolResult} {attachments} />
				{:else}
					<div class="overflow-auto">
						{#each parsedLines as line, i (i)}
							<div class="font-mono text-[11px] leading-relaxed whitespace-pre-wrap">
								{line.text}
							</div>
							{#if line.image}
								<img
									src={line.image.base64Url}
									alt={line.image.name}
									class="mt-2 mb-2 h-auto max-w-full rounded-lg"
									loading="lazy"
								/>
							{/if}
						{/each}
					</div>
				{/if}
			{:else}
				<div class="rounded bg-muted/20 p-2 text-xs text-muted-foreground/70 italic">{t('No output')}</div>
			{/if}
		{/if}
	{/snippet}
</ToolCallBlock>
