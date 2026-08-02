<script>let { title, children } = $props();
export {};
</script>

<div>
	<h3 class="mb-6 text-base font-semibold">{title}</h3>

	<div class="space-y-8">
		{@render children()}
	</div>
</div>
