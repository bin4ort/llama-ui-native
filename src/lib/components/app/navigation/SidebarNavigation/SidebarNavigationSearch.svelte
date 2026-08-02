<script>import { t } from '$lib/stores/i18n.svelte';
let { value = $bindable(''), placeholder = t("Search conversations..."), onInput, class: className } = $props();
</script>

<div class="mb-4 px-2 {className}">
	<SearchInput bind:value {placeholder} {onInput} />
</div>
