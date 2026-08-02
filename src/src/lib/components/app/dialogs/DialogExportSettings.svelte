<script lang="ts">
	import * as AlertDialog from '$lib/components/ui/alert-dialog';
	import { Checkbox } from '$lib/components/ui/checkbox';
	import Label from '$lib/components/ui/label/label.svelte';
	import { Shield, ShieldOff } from '@lucide/svelte';
	import { t, tr } from '$lib/stores/i18n.svelte';

	let {
		open = $bindable(),
		includeSensitiveData = $bindable(false),
		onCancel,
		onConfirm
	}: {
		open: boolean;
		includeSensitiveData: boolean;
		onCancel: () => void;
		onConfirm: () => void;
	} = $props();

	function handleOpenChange(newOpen: boolean) {
		if (!newOpen) {
			onCancel();
		}
	}
</script>

<AlertDialog.Root {open} onOpenChange={handleOpenChange}>
	<AlertDialog.Content>
		<AlertDialog.Header>
			<AlertDialog.Title class="flex items-center gap-2">
				{#if includeSensitiveData}
					<ShieldOff class="h-5 w-5 text-destructive" />
				{:else}
					<Shield class="h-5 w-5 text-destructive" />
				{/if}
				{tr.dict["Export Settings"] || "Export Settings"}
			</AlertDialog.Title>

			<AlertDialog.Description>
				{#if includeSensitiveData}
					<p class="text-amber-500">
						{tr.dict["Warning: This export will include sensitive data such as API keys and MCP server custom headers (e.g., authorization tokens). Do not share this file with anyone you don't trust."] || "Warning: This export will include sensitive data such as API keys and MCP server custom headers (e.g., authorization tokens). Do not share this file with anyone you don't trust."}
					</p>
				{:else}
					<p>
						{tr.dict["Sensitive data (API keys, MCP server custom headers) will not be included in the export to protect your credentials."] || "Sensitive data (API keys, MCP server custom headers) will not be included in the export to protect your credentials."}
					</p>
				{/if}
			</AlertDialog.Description>
		</AlertDialog.Header>

		<div class="flex items-center gap-2 py-2">
			<Checkbox id="include-sensitive" bind:checked={includeSensitiveData} />

			<Label
				for="include-sensitive"
				class="text-sm leading-none peer-disabled:cursor-not-allowed peer-disabled:opacity-70"
			>
				{#if includeSensitiveData}
					<span class="text-destructive">{tr.dict["Include sensitive data (not recommended)"] || "Include sensitive data (not recommended)"}</span>
				{:else}
					<span>{tr.dict["Include sensitive data"] || "Include sensitive data"}</span>
				{/if}
			</Label>
		</div>

		<AlertDialog.Footer>
			<AlertDialog.Cancel onclick={onCancel}>{tr.dict["Cancel"] || "Cancel"}</AlertDialog.Cancel>

			<AlertDialog.Action
				onclick={onConfirm}
				class="bg-destructive text-white hover:bg-destructive/80"
			>
				{#if includeSensitiveData}
					{tr.dict["Export Anyway"] || "Export Anyway"}
				{:else}
					{tr.dict["Export Without Sensitive Data"] || "Export Without Sensitive Data"}
				{/if}
			</AlertDialog.Action>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
