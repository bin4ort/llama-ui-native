<script>let { displayTextContent, language } = $props();
export {};
</script>

{#if displayTextContent}
	<div class="px-4 pb-4">
		<SyntaxHighlightedCode
			class="max-w-4xl"
			code={displayTextContent}
			{language}
			maxHeight="none"
		/>
	</div>
{/if}
