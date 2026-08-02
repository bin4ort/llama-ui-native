import { AUTO_SCROLL_AT_BOTTOM_THRESHOLD, AUTO_SCROLL_INTERVAL } from '$lib/constants';
/**
 * Creates an auto-scroll controller for a scrollable container.
 *
 * Features:
 * - Auto-scrolls to bottom during streaming/loading
 * - Stops auto-scroll when user manually scrolls up
 * - Resumes auto-scroll when user scrolls back to bottom
 */
export class AutoScrollController {
    _autoScrollEnabled = $state(true);
    _userScrolledUp = $state(false);
    _lastScrollTop = $state(0);
    _scrollInterval;
    _container;
    _disabled;
    _mutationObserver = null;
    _rafPending = false;
    _observerEnabled = false;
    constructor(options = {}) {
        this._disabled = options.disabled ?? false;
    }
    get autoScrollEnabled() {
        return this._autoScrollEnabled;
    }
    get userScrolledUp() {
        return this._userScrolledUp;
    }
    /**
     * Binds the controller to a scrollable container element.
     */
    setContainer(container) {
        this._doStopObserving();
        this._container = container;
        if (this._observerEnabled && container && !this._disabled) {
            this._doStartObserving();
        }
    }
    /**
     * Updates the disabled state.
     */
    setDisabled(disabled) {
        if (this._disabled === disabled)
            return;
        this._disabled = disabled;
        if (disabled) {
            this._autoScrollEnabled = false;
            this.stopInterval();
            this._doStopObserving();
        }
        else if (this._observerEnabled && this._container && !this._mutationObserver) {
            this._doStartObserving();
        }
    }
    /**
     * Handles scroll events to detect user scroll direction and toggle auto-scroll.
     */
    handleScroll() {
        if (this._disabled || !this._container)
            return;
        const { scrollTop, scrollHeight, clientHeight } = this._container;
        const distanceFromBottom = scrollHeight - clientHeight - scrollTop;
        const isScrollingUp = scrollTop < this._lastScrollTop;
        const isAtBottom = distanceFromBottom < AUTO_SCROLL_AT_BOTTOM_THRESHOLD;
        if (isScrollingUp && !isAtBottom) {
            this._userScrolledUp = true;
            this._autoScrollEnabled = false;
        }
        else if (isAtBottom && this._userScrolledUp) {
            this._userScrolledUp = false;
            this._autoScrollEnabled = true;
        }
        this._lastScrollTop = scrollTop;
    }
    /**
     * Scrolls the container to the bottom instantly.
     */
    scrollToBottom() {
        if (this._disabled || !this._container)
            return;
        this._container.scrollTop = this._container.scrollHeight;
    }
    /**
     * Enables auto-scroll (e.g., when user sends a message).
     */
    enable() {
        if (this._disabled)
            return;
        this._userScrolledUp = false;
        this._autoScrollEnabled = true;
    }
    /**
     * Resets scroll state when switching conversations.
     */
    resetScrollState() {
        this._userScrolledUp = false;
        this._autoScrollEnabled = !this._disabled;
        if (this._container) {
            this._lastScrollTop = this._container.scrollTop;
        }
    }
    /**
     * Starts the auto-scroll interval for continuous scrolling during streaming.
     */
    startInterval() {
        if (this._disabled || this._scrollInterval)
            return;
        this._scrollInterval = setInterval(() => {
            this.scrollToBottom();
        }, AUTO_SCROLL_INTERVAL);
    }
    /**
     * Stops the auto-scroll interval.
     */
    stopInterval() {
        if (this._scrollInterval) {
            clearInterval(this._scrollInterval);
            this._scrollInterval = undefined;
        }
    }
    /**
     * Updates the auto-scroll interval based on streaming state.
     * Call this in a $effect to automatically manage the interval.
     */
    updateInterval(isStreaming) {
        if (this._disabled) {
            this.stopInterval();
            return;
        }
        if (isStreaming && this._autoScrollEnabled) {
            if (!this._scrollInterval) {
                this.startInterval();
            }
        }
        else {
            this.stopInterval();
        }
    }
    /**
     * Cleans up resources. Call this in onDestroy or when the component unmounts.
     */
    destroy() {
        this.stopInterval();
        this._doStopObserving();
    }
    /**
     * Starts a MutationObserver on the container that auto-scrolls to bottom
     * on content changes. More responsive than interval-based polling.
     */
    startObserving() {
        this._observerEnabled = true;
        if (this._container && !this._disabled && !this._mutationObserver) {
            this._doStartObserving();
        }
    }
    /**
     * Stops the MutationObserver.
     */
    stopObserving() {
        this._observerEnabled = false;
        this._doStopObserving();
    }
    _doStartObserving() {
        if (!this._container || this._mutationObserver)
            return;
        this._mutationObserver = new MutationObserver(() => {
            if (!this._autoScrollEnabled || this._rafPending)
                return;
            this._rafPending = true;
            requestAnimationFrame(() => {
                this._rafPending = false;
                if (this._autoScrollEnabled && this._container) {
                    this._container.scrollTop = this._container.scrollHeight;
                }
            });
        });
        this._mutationObserver.observe(this._container, {
            childList: true,
            subtree: true,
            characterData: true
        });
    }
    _doStopObserving() {
        if (this._mutationObserver) {
            this._mutationObserver.disconnect();
            this._mutationObserver = null;
        }
        this._rafPending = false;
    }
}
/**
 * Creates a new AutoScrollController instance.
 */
export function createAutoScrollController(options = {}) {
    return new AutoScrollController(options);
}
