<script>let { isEmpty = false } = $props();
export {};
</script>

<div
	class={[
		'pointer-events-none mb-4 hidden px-4 text-center text-balance',
		isEmpty && 'mb-[calc(50dvh-8rem)] md:mb-6 pointer-events-auto block!'
	]}
>
	<h1 class="mb-2 text-2xl font-semibold tracking-tight md:text-3xl">{t("Hello there")}</h1>

	<p class="text-muted-foreground md:text-lg">
		{serverStore.props?.modalities?.audio ? t('Record audio, type a message ') : t('Type a message')} {t('or upload files to get started')}
	</p>
</div>
