import { SETTINGS_FALLBACK_EXIT_ROUTE } from '$lib/constants';
let _url = $state(SETTINGS_FALLBACK_EXIT_ROUTE);
export const settingsReferrer = {
    get url() {
        return _url;
    },
    set url(value) {
        _url = value;
    }
};
