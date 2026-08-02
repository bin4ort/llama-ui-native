<script>import { KeyboardKey } from '$lib/enums';
let { open = $bindable(), title, description, confirmText = 'Confirm', cancelText = 'Cancel', variant = 'default', icon, onConfirm, onCancel, onKeydown, children } = $props();
function handleKeydown(event) {
    if (event.key === KeyboardKey.ENTER) {
        event.preventDefault();
        onConfirm();
    }
    onKeydown?.(event);
}
function handleOpenChange(newOpen) {
    if (!newOpen) {
        onCancel();
    }
}
</script>

<AlertDialog.Root {open} onOpenChange={handleOpenChange}>
	<AlertDialog.Content onkeydown={handleKeydown}>
		<AlertDialog.Header>
			<AlertDialog.Title class="flex items-center gap-2">
				{#if icon}
					{@const IconComponent = icon}

					<IconComponent class="h-5 w-5 {variant === 'destructive' ? 'text-destructive' : ''}" />
				{/if}
				{title}
			</AlertDialog.Title>

			<AlertDialog.Description>
				{description}
			</AlertDialog.Description>
		</AlertDialog.Header>

		{#if children}
			{@render children()}
		{/if}

		<AlertDialog.Footer>
			<AlertDialog.Cancel onclick={onCancel}>{cancelText}</AlertDialog.Cancel>
			<AlertDialog.Action
				onclick={onConfirm}
				class={variant === 'destructive' ? 'bg-destructive text-white hover:bg-destructive/80' : ''}
			>
				{confirmText}
			</AlertDialog.Action>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
