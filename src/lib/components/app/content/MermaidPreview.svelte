<script>import { mountSvgShadow } from '$lib/utils/svg-shadow';
import { SVG_DIALOG_SHADOW_STYLE } from '$lib/constants';
let { svgHtml } = $props();
let svgHost = $state(null);
// Re-mount on every svgHtml change so a live streaming svg keeps rendering while zoomed
$effect(() => {
    if (svgHost)
        mountSvgShadow(svgHost, svgHtml, SVG_DIALOG_SHADOW_STYLE);
});
// Zoom and pan state
let scale = $state(1);
let translateX = $state(0);
let translateY = $state(0);
let isDragging = $state(false);
const containerRef = { current: null };
// Drag start position
let dragStartX = 0;
let dragStartY = 0;
let dragStartTranslateX = 0;
let dragStartTranslateY = 0;
const MIN_SCALE = 0.1;
const MAX_SCALE = 10;
const ZOOM_STEP = 0.15;
function resetView() {
    scale = 1;
    translateX = 0;
    translateY = 0;
}
function zoomIn() {
    scale = Math.min(scale + ZOOM_STEP, MAX_SCALE);
}
function zoomOut() {
    scale = Math.max(scale - ZOOM_STEP, MIN_SCALE);
}
function handleWheel(event) {
    event.preventDefault();
    const delta = event.deltaY > 0 ? -ZOOM_STEP : ZOOM_STEP;
    scale = Math.min(Math.max(scale + delta, MIN_SCALE), MAX_SCALE);
}
// Imperatively attach a non-passive wheel listener so preventDefault() actually works
// (Svelte 5 wheel listeners are passive by default, making preventDefault() a no-op)
$effect(() => {
    const el = containerRef.current;
    if (!el)
        return;
    function onWheel(e) {
        handleWheel(e);
    }
    el.addEventListener('wheel', onWheel, { passive: false });
    return () => el.removeEventListener('wheel', onWheel);
});
function handlePointerDown(event) {
    if (event.button !== 0 && event.pointerType === 'mouse')
        return;
    isDragging = true;
    dragStartX = event.clientX;
    dragStartY = event.clientY;
    dragStartTranslateX = translateX;
    dragStartTranslateY = translateY;
    event.currentTarget.setPointerCapture(event.pointerId);
}
function handlePointerMove(event) {
    if (!isDragging)
        return;
    translateX = dragStartTranslateX + (event.clientX - dragStartX);
    translateY = dragStartTranslateY + (event.clientY - dragStartY);
}
function handlePointerUp() {
    isDragging = false;
}
</script>

<div
	bind:this={containerRef.current}
	class="mermaid-preview relative flex items-center justify-center overflow-hidden bg-muted/20"
>
	<!-- svelte-ignore a11y_no_static_element_interactions -->
	<div
		class="mermaid-preview-diagram transform-origin-center inline-block min-h-fit min-w-fit will-change-transform {isDragging &&
			'select-none'}"
		style="transform: translate({translateX}px, {translateY}px) scale({scale}); cursor: {isDragging
			? 'grabbing'
			: 'grab'};"
		onpointerdown={handlePointerDown}
		onpointermove={handlePointerMove}
		onpointerup={handlePointerUp}
		onpointerleave={handlePointerUp}
	>
		<div bind:this={svgHost}></div>
	</div>

	<MermaidPreviewControls
		{scale}
		{svgHtml}
		onZoomIn={zoomIn}
		onZoomOut={zoomOut}
		onResetView={resetView}
	/>
</div>
