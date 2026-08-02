/**
 * Event handler factories for markdown content components.
 * Uses dependency injection pattern to avoid direct component state access.
 */
import { copyCodeToClipboard, copyToClipboard } from '$lib/utils';
import { MERMAID_WRAPPER_CLASS, MERMAID_BLOCK_CLASS, MERMAID_SYNTAX_ATTR } from '$lib/constants';
/**
 * Creates a click handler for copy buttons in code blocks.
 * Copies the code content to clipboard.
 */
export function createHandleCopyClick() {
    return async function handleCopyClick(event) {
        event.preventDefault();
        event.stopPropagation();
        const target = event.currentTarget;
        if (!target)
            return;
        const wrapper = target.closest('.code-block-wrapper');
        if (!wrapper)
            return;
        const codeElement = wrapper.querySelector('code[data-code-id]');
        if (!codeElement)
            return;
        const rawCode = codeElement.textContent ?? '';
        try {
            await copyCodeToClipboard(rawCode);
        }
        catch (error) {
            console.error('Failed to copy code:', error);
        }
    };
}
/**
 * Creates a handler for preview dialog open state changes.
 * Clears preview content when dialog is closed.
 */
export function createHandlePreviewDialogOpenChange(previewState) {
    return function handlePreviewDialogOpenChange(open) {
        previewState.setPreviewDialogOpen(open);
        if (!open) {
            previewState.setPreviewCode('');
            previewState.setPreviewLanguage('text');
        }
    };
}
/**
 * Creates a click handler for preview buttons within HTML code blocks.
 * Opens a preview dialog with the rendered HTML content.
 */
export function createHandlePreviewClick(previewState) {
    return async function handlePreviewClick(event) {
        event.preventDefault();
        event.stopPropagation();
        const target = event.currentTarget;
        if (!target)
            return;
        const wrapper = target.closest('.code-block-wrapper');
        if (!wrapper)
            return;
        const codeElement = wrapper.querySelector('code[data-code-id]');
        if (!codeElement)
            return;
        const rawCode = codeElement.textContent ?? '';
        const languageLabel = wrapper.querySelector('.code-language');
        const language = languageLabel?.textContent?.trim() || 'text';
        previewState.setPreviewCode(rawCode);
        previewState.setPreviewLanguage(language);
        previewState.setPreviewDialogOpen(true);
    };
}
/**
 * Creates a click handler for mermaid block interactions.
 * Handles copy, preview, and diagram click events via event delegation.
 */
export function createHandleMermaidClick(mermaidState) {
    return async function handleMermaidClick(event) {
        const target = event.target;
        // Check if clicking on copy or preview button in mermaid block
        const copyBtn = target.closest(`.${MERMAID_WRAPPER_CLASS} .copy-code-btn`);
        const previewBtn = target.closest(`.${MERMAID_WRAPPER_CLASS} .preview-code-btn`);
        if (copyBtn || previewBtn) {
            const wrapper = target.closest(`.${MERMAID_WRAPPER_CLASS}`);
            if (!wrapper)
                return;
            const preElement = wrapper.querySelector(`pre.${MERMAID_BLOCK_CLASS}[${MERMAID_SYNTAX_ATTR}]`);
            if (!preElement)
                return;
            const mermaidSyntax = preElement.getAttribute(MERMAID_SYNTAX_ATTR) ?? '';
            if (copyBtn) {
                event.preventDefault();
                event.stopPropagation();
                try {
                    await copyToClipboard(mermaidSyntax);
                }
                catch (error) {
                    console.error('Failed to copy mermaid syntax:', error);
                }
                return;
            }
            if (previewBtn) {
                event.preventDefault();
                event.stopPropagation();
                const svg = preElement.querySelector('svg');
                if (!svg)
                    return;
                mermaidState.setMermaidPreviewSvgHtml(svg.outerHTML);
                mermaidState.setMermaidPreviewOpen(true);
                return;
            }
        }
        // Otherwise, open preview when clicking on the mermaid diagram itself
        const mermaidEl = target.closest(`.${MERMAID_BLOCK_CLASS}`);
        if (!mermaidEl)
            return;
        const svg = mermaidEl.querySelector('svg');
        if (!svg)
            return;
        mermaidState.setMermaidPreviewSvgHtml(svg.outerHTML);
        mermaidState.setMermaidPreviewOpen(true);
    };
}
/**
 * Creates a handler for mermaid preview dialog open state changes.
 * Cleans up SVG content when dialog is closed.
 */
export function createHandleMermaidPreviewOpenChange(mermaidState) {
    return function handleMermaidPreviewOpenChange(open) {
        mermaidState.setMermaidPreviewOpen(open);
        if (!open) {
            mermaidState.setMermaidPreviewSvgHtml('');
        }
    };
}
/**
 * Creates an error handler for images that fail to load (e.g., CORS issues).
 * Shows fallback UI for broken images.
 */
export function createHandleImageError(renderedBlocksState, IMAGE_NOT_ERROR_BOUND_SELECTOR, DATA_ERROR_BOUND_ATTR, BOOL_TRUE_STRING) {
    return async function handleImageError(event) {
        const img = event.target;
        if (!img)
            return;
        const blockId = img.closest('[data-block-id]')?.getAttribute('data-block-id');
        if (!blockId)
            return;
        const block = renderedBlocksState.renderedBlocks.find((b) => b.id === blockId);
        if (!block)
            return;
        // Skip if already handled
        if (img.dataset[DATA_ERROR_BOUND_ATTR] === BOOL_TRUE_STRING)
            return;
        img.dataset[DATA_ERROR_BOUND_ATTR] = BOOL_TRUE_STRING;
        // Get the fallback HTML and replace the image
        const fallbackHtml = `<div class="image-error-placeholder" data-original-src="${img.src}">
			<span class="image-error-icon">⚠️</span>
			<span class="image-error-text">Failed to load image</span>
		</div>`;
        // Replace the img element with fallback in the block's HTML
        const newHtml = block.html.replace(/img[^>]*src=["']([^"']*)[^>]*>/g, (match, src) => {
            if (src === img.src) {
                return fallbackHtml.replace('data-original-src=""', `data-original-src="${src}"`);
            }
            return match;
        });
        // Update the block
        const newBlocks = renderedBlocksState.renderedBlocks.map((b) => b.id === blockId ? { ...b, html: newHtml } : b);
        renderedBlocksState.setRenderedBlocks(newBlocks);
    };
}
/**
 * Creates a function to set up code block action event listeners.
 * Binds click handlers to copy and preview buttons within code blocks.
 */
export function createSetupCodeBlockActions(handleCopyClick, handlePreviewClick) {
    return function setupCodeBlockActions(containerRef) {
        if (!containerRef)
            return;
        const wrappers = containerRef.querySelectorAll('.code-block-wrapper');
        for (const wrapper of wrappers) {
            const copyButton = wrapper.querySelector('.copy-code-btn');
            const previewButton = wrapper.querySelector('.preview-code-btn');
            if (copyButton && copyButton.dataset.listenerBound !== 'true') {
                copyButton.dataset.listenerBound = 'true';
                copyButton.addEventListener('click', handleCopyClick);
            }
            if (previewButton && previewButton.dataset.listenerBound !== 'true') {
                previewButton.dataset.listenerBound = 'true';
                previewButton.addEventListener('click', handlePreviewClick);
            }
        }
    };
}
/**
 * Creates a function to set up image error handlers.
 * Attaches error handlers to images to show fallback UI when loading fails.
 */
export function createSetupImageErrorHandlers(handleImageError, IMAGE_NOT_ERROR_BOUND_SELECTOR, DATA_ERROR_BOUND_ATTR, BOOL_TRUE_STRING) {
    return function setupImageErrorHandlers(containerRef) {
        if (!containerRef)
            return;
        const images = containerRef.querySelectorAll(IMAGE_NOT_ERROR_BOUND_SELECTOR);
        for (const img of images) {
            img.dataset[DATA_ERROR_BOUND_ATTR] = BOOL_TRUE_STRING;
            img.addEventListener('error', handleImageError);
        }
    };
}
