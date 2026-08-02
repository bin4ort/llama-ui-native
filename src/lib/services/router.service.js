import { ROUTES } from '$lib/constants/routes';
export class RouterService {
    static chat(id) {
        return `${ROUTES.CHAT}/${id}`;
    }
    static settings(section) {
        return `${ROUTES.SETTINGS}/${section}`;
    }
}
