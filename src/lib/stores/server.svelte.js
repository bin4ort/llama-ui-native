import { PropsService } from '$lib/services/props.service';
import { ServerRole } from '$lib/enums';
import { ApiError } from '$lib/utils/api-fetch';
const LOADING_RETRY_INTERVAL_MS = 1000;
/**
 * serverStore - Server connection state, configuration, and role detection
 *
 * This store manages the server connection state and properties fetched from `/props`.
 * It provides reactive state for server configuration and role detection.
 *
 * **Architecture & Relationships:**
 * - **PropsService**: Stateless service for fetching `/props` data
 * - **serverStore** (this class): Reactive store for server state
 * - **modelsStore**: Independent store for model management (uses PropsService directly)
 *
 * **Key Features:**
 * - **Server State**: Connection status, loading, error handling
 * - **Role Detection**: MODEL (single model) vs ROUTER (multi-model)
 * - **Default Params**: Server-wide generation defaults
 */
class ServerStore {
    /**
     *
     *
     * State
     *
     *
     */
    props = $state(null);
    loading = $state(false);
    error = $state(null);
    status = $state(null);
    role = $state(null);
    fetchPromise = null;
    retryTimer = null;
    /**
     *
     *
     * Getters
     *
     *
     */
    get defaultParams() {
        return this.props?.default_generation_settings?.params || null;
    }
    get contextSize() {
        const nCtx = this.props?.default_generation_settings?.n_ctx;
        return typeof nCtx === 'number' ? nCtx : null;
    }
    get uiSettings() {
        return this.props?.ui_settings ?? this.props?.webui_settings;
    }
    get isRouterMode() {
        return this.role === ServerRole.ROUTER;
    }
    get isModelMode() {
        return this.role === ServerRole.MODEL;
    }
    /**
     *
     *
     * Data Handling
     *
     *
     */
    /**
     * @param background - Set by the automatic "still loading" poll. Skips the
     * `loading` flag flip so the UI doesn't bounce between the full loading
     * splash and the chat screen every retry tick.
     */
    async fetch({ background = false } = {}) {
        if (this.fetchPromise)
            return this.fetchPromise;
        this.clearRetryTimer();
        if (!background) {
            this.loading = true;
        }
        // Don't clear an existing "still loading" error before a retry -
        // doing so would unmount/remount the error banner every second.
        if (this.status !== 503) {
            this.error = null;
        }
        const fetchPromise = (async () => {
            try {
                const props = await PropsService.fetch();
                this.props = props;
                this.error = null;
                this.status = null;
                this.detectRole(props);
            }
            catch (error) {
                this.error = error instanceof Error ? error.message : String(error);
                this.status = error instanceof ApiError ? error.status : null;
                console.error('Error fetching server properties:', error);
                if (this.status === 503) {
                    this.scheduleRetry();
                }
            }
            finally {
                if (!background) {
                    this.loading = false;
                }
                this.fetchPromise = null;
            }
        })();
        this.fetchPromise = fetchPromise;
        await fetchPromise;
    }
    clear() {
        this.clearRetryTimer();
        this.props = null;
        this.error = null;
        this.status = null;
        this.loading = false;
        this.role = null;
        this.fetchPromise = null;
    }
    scheduleRetry() {
        if (this.retryTimer)
            return;
        this.retryTimer = setTimeout(() => {
            this.retryTimer = null;
            this.fetch({ background: true });
        }, LOADING_RETRY_INTERVAL_MS);
    }
    clearRetryTimer() {
        if (this.retryTimer) {
            clearTimeout(this.retryTimer);
            this.retryTimer = null;
        }
    }
    /**
     *
     *
     * Utilities
     *
     *
     */
    detectRole(props) {
        const newRole = props?.role === ServerRole.ROUTER ? ServerRole.ROUTER : ServerRole.MODEL;
        if (this.role !== newRole) {
            this.role = newRole;
            console.info(`Server running in ${newRole === ServerRole.ROUTER ? 'ROUTER' : 'MODEL'} mode`);
        }
    }
}
export const serverStore = new ServerStore();
export const serverProps = () => serverStore.props;
export const serverLoading = () => serverStore.loading;
export const serverError = () => serverStore.error;
export const serverStatus = () => serverStore.status;
export const serverRole = () => serverStore.role;
export const defaultParams = () => serverStore.defaultParams;
export const contextSize = () => serverStore.contextSize;
export const isRouterMode = () => serverStore.isRouterMode;
export const isModelMode = () => serverStore.isModelMode;
