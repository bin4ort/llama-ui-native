<script lang="ts">
	import * as AlertDialog from '$lib/components/ui/alert-dialog';
	import { t, tr } from '$lib/stores/i18n.svelte';

	interface Props {
		open: boolean;
		fileErrorData: {
			generallyUnsupported: File[];
			modalityUnsupported: File[];
			modalityReasons: Record<string, string>;
			supportedTypes: string[];
		};
		onOpenChange?: (open: boolean) => void;
	}

	let { open = $bindable(), fileErrorData, onOpenChange }: Props = $props();

	function handleOpenChange(newOpen: boolean) {
		open = newOpen;

		onOpenChange?.(newOpen);
	}
</script>

<AlertDialog.Root {open} onOpenChange={handleOpenChange}>
	<AlertDialog.Portal>
		<AlertDialog.Overlay />

		<AlertDialog.Content class="flex max-w-md flex-col">
			<AlertDialog.Header>
				<AlertDialog.Title>{tr.dict["File Upload Error"] || "File Upload Error"}</AlertDialog.Title>

				<AlertDialog.Description class="text-sm text-muted-foreground">
					{tr.dict["Some files cannot be uploaded with the current model."] || "Some files cannot be uploaded with the current model."}
				</AlertDialog.Description>
			</AlertDialog.Header>

			<div class="!max-h-[50vh] min-h-0 flex-1 space-y-4 overflow-y-auto">
				{#if fileErrorData.generallyUnsupported.length > 0}
					<div class="space-y-2">
						<h4 class="text-sm font-medium text-destructive">{tr.dict["Unsupported File Types"] || "Unsupported File Types"}</h4>

						<div class="space-y-1">
							{#each fileErrorData.generallyUnsupported as file (file.name)}
								<div class="rounded-md bg-destructive/10 px-3 py-2">
									<p class="font-mono text-sm break-all text-destructive">
										{file.name}
									</p>

									<p class="mt-1 text-xs text-muted-foreground">{tr.dict["File type not supported"] || "File type not supported"}</p>
								</div>
							{/each}
						</div>
					</div>
				{/if}

				{#if fileErrorData.modalityUnsupported.length > 0}
					<div class="space-y-2">
						<div class="space-y-1">
							{#each fileErrorData.modalityUnsupported as file (file.name)}
								<div class="rounded-md bg-destructive/10 px-3 py-2">
									<p class="font-mono text-sm break-all text-destructive">
										{file.name}
									</p>

									<p class="mt-1 text-xs text-muted-foreground">
										{fileErrorData.modalityReasons[file.name] || t('Not supported by current model')}
									</p>
								</div>
							{/each}
						</div>
					</div>
				{/if}
			</div>

			<div class="rounded-md bg-muted/50 p-3">
				<h4 class="mb-2 text-sm font-medium">{tr.dict["This model supports:"] || "This model supports:"}</h4>

				<p class="text-sm text-muted-foreground">
					{fileErrorData.supportedTypes.join(', ')}
				</p>
			</div>

			<AlertDialog.Footer>
				<AlertDialog.Action onclick={() => handleOpenChange(false)}>{tr.dict["Got it"] || "Got it"}</AlertDialog.Action>
			</AlertDialog.Footer>
		</AlertDialog.Content>
	</AlertDialog.Portal>
</AlertDialog.Root>
