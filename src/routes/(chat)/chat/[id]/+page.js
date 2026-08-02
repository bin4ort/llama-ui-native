
import { validateApiKey } from '$lib/utils';

export const load = async ({ fetch }) => {
	await validateApiKey(fetch);
};
