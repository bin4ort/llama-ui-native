<script>let { onDecision } = $props();
export {};
</script>

<ChatMessageActionCard icon={RotateCw}>
	{#snippet message()}
		Agentic turn limit reached. Continue?
	{/snippet}

	{#snippet actions()}
		<Button size="sm" onclick={() => onDecision(true)}>{t('Continue')}</Button>

		<Button
			variant="destructive"
			size="sm"
			class="text-destructive hover:text-destructive"
			onclick={() => onDecision(false)}
		>
			{t("Stop")}
		</Button>
	{/snippet}
</ChatMessageActionCard>
