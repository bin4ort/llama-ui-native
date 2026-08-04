<script lang="ts">
	import { t } from '$lib/stores/i18n.svelte';
	import { verificationStore, resolveVerification, cancelVerification } from '$lib/stores/verification.svelte';
	import * as AlertDialog from '$lib/components/ui/alert-dialog';
	import { Button } from '$lib/components/ui/button';
	import { Loader2, ShieldAlert } from '@lucide/svelte';

	let open = $derived(!!verificationStore.pending);
	let verifying = $state(false);

	async function handleConfirm() {
		const request = verificationStore.pending;
		if (!request) return;
		if (request.verify) {
			verifying = true;
			const ok = await request.verify();
			verifying = false;
			resolveVerification(ok);
		} else {
			resolveVerification(true);
		}
	}

	function handleCancel() {
		cancelVerification();
	}
</script>

<AlertDialog.Root open={open} onOpenChange={(o) => { if (!o && !verifying) cancelVerification(); }}>
	<AlertDialog.Portal>
		<AlertDialog.Overlay class="z-9999" />
		<AlertDialog.Content class="z-9999" onkeydown={(e) => e.key === 'Escape' && handleCancel()}>
			<AlertDialog.Header>
				<AlertDialog.Title class="flex items-center gap-2">
					<ShieldAlert class="h-5 w-5 text-amber-400" />
					{verificationStore.pending?.title ?? ''}
				</AlertDialog.Title>
				<AlertDialog.Description class="whitespace-pre-wrap">
					{verificationStore.pending?.description ?? ''}
				</AlertDialog.Description>
			</AlertDialog.Header>

			<AlertDialog.Footer>
				<Button variant="outline" onclick={handleCancel} disabled={verifying}>
					{verificationStore.pending?.cancelText ?? t('Cancel')}
				</Button>
				<Button onclick={handleConfirm} disabled={verifying}>
					{#if verifying}
						<Loader2 class="mr-2 h-4 w-4 animate-spin" />
						{verificationStore.pending?.verifyingText ?? t('Verifying…')}
					{:else}
						{verificationStore.pending?.confirmText ?? t('Confirm')}
					{/if}
				</Button>
			</AlertDialog.Footer>
		</AlertDialog.Content>
	</AlertDialog.Portal>
</AlertDialog.Root>
