/**
 * VerificationStore — modular safety-verification prompt.
 *
 * Any feature can request a one-shot verification dialog before a risky
 * action (e.g. enabling "always allow" tool permissions, later: unlocking
 * the conversation vault). The dialog resolves the promise with true/false.
 * Optional `verify` runs an async check (e.g. a password) before resolving.
 */
export interface VerificationRequest {
	/** Pre-translated title. */
	title: string;
	/** Pre-translated description body. */
	description: string;
	/** Pre-translated confirm button label (default: caller-provided). */
	confirmText?: string;
	/** Pre-translated cancel button label (default: caller-provided). */
	cancelText?: string;
	/** Pre-translated label shown while `verify` is running. */
	verifyingText?: string;
	/** Optional async check; the dialog resolves with its result. */
	verify?: () => Promise<boolean>;
}

let pendingRequest = $state<VerificationRequest | null>(null);
let resolveFn = $state<((ok: boolean) => void) | null>(null);

export function requestVerification(request: VerificationRequest): Promise<boolean> {
	return new Promise<boolean>((resolve) => {
		pendingRequest = request;
		resolveFn = resolve;
	});
}

export function resolveVerification(ok: boolean): void {
	resolveFn?.(ok);
	resolveFn = null;
	pendingRequest = null;
}

export function cancelVerification(): void {
	resolveFn?.(false);
	resolveFn = null;
	pendingRequest = null;
}

export const verificationStore = {
	get pending(): VerificationRequest | null {
		return pendingRequest;
	}
};
