<script lang="ts">
	import * as AlertDialog from '$lib/components/ui/alert-dialog';
	import { FileX } from '@lucide/svelte';
	import { t, tr } from '$lib/stores/i18n.svelte';

	interface Props {
		open: boolean;
		emptyFiles: string[];
		onOpenChange?: (open: boolean) => void;
	}

	let { open = $bindable(), emptyFiles, onOpenChange }: Props = $props();

	function handleOpenChange(newOpen: boolean) {
		open = newOpen;
		onOpenChange?.(newOpen);
	}
</script>

<AlertDialog.Root {open} onOpenChange={handleOpenChange}>
	<AlertDialog.Content>
		<AlertDialog.Header>
			<AlertDialog.Title class="flex items-center gap-2">
				<FileX class="h-5 w-5 text-destructive" />

				{tr.dict["Empty Files Detected"] || "Empty Files Detected"}
			</AlertDialog.Title>

			<AlertDialog.Description>
				{tr.dict["The following files are empty and have been removed from your attachments:"] || "The following files are empty and have been removed from your attachments:"}
			</AlertDialog.Description>
		</AlertDialog.Header>

		<div class="space-y-3 text-sm">
			<div class="rounded-lg bg-muted p-3">
				<div class="mb-2 font-medium">{tr.dict["Empty Files:"] || "Empty Files:"}</div>

				<ul class="list-inside list-disc space-y-1 text-muted-foreground">
					{#each emptyFiles as fileName (fileName)}
						<li class="font-mono text-sm">{fileName}</li>
					{/each}
				</ul>
			</div>

			<div>
				<div class="mb-2 font-medium">{tr.dict["What happened:"] || "What happened:"}</div>

				<ul class="list-inside list-disc space-y-1 text-muted-foreground">
					<li>{tr.dict["Empty files cannot be processed or sent to the AI model"] || "Empty files cannot be processed or sent to the AI model"}</li>

					<li>{tr.dict["These files have been automatically removed from your attachments"] || "These files have been automatically removed from your attachments"}</li>

					<li>{tr.dict["You can try uploading files with content instead"] || "You can try uploading files with content instead"}</li>
				</ul>
			</div>
		</div>

		<AlertDialog.Footer>
			<AlertDialog.Action onclick={() => handleOpenChange(false)}>{tr.dict["Got it"] || "Got it"}</AlertDialog.Action>
		</AlertDialog.Footer>
	</AlertDialog.Content>
</AlertDialog.Root>
