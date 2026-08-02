<script>let { class: className = '', multiple = true, onFileSelect } = $props();
let fileInputElement;
export function click() {
    fileInputElement?.click();
}
function handleFileSelect(event) {
    const input = event.target;
    if (input.files) {
        onFileSelect?.(Array.from(input.files));
    }
}
</script>

<input
	bind:this={fileInputElement}
	type="file"
	{multiple}
	onchange={handleFileSelect}
	class="hidden {className}"
/>
