<script>import { Image, Music, Video, FileText, FileIcon } from '@lucide/svelte';
let { currentItem, isImage, isAudio, isVideo, isPdf, isText, displayPreview, displayTextContent, audioSrc, videoSrc, language, hasVisionModality, activeModelId } = $props();
let IconComponent = $derived(isImage ? Image : isText || isPdf ? FileText : isAudio ? Music : isVideo ? Video : FileIcon);
let isUnavailable = $derived(!isPdf && !isImage && !(isText && displayTextContent) && !isAudio && !isVideo);
</script>

{#if currentItem}
	{#key currentItem.id}
		{#if isPdf}
			<ChatAttachmentsPreviewCurrentItemPdf
				{currentItem}
				displayName={currentItem.name}
				{displayTextContent}
				{hasVisionModality}
				{activeModelId}
			/>
		{:else if isImage}
			<ChatAttachmentsPreviewCurrentItemImage {currentItem} {displayPreview} />
		{:else if isText && displayTextContent}
			<ChatAttachmentsPreviewCurrentItemText {displayTextContent} {language} />
		{:else if isAudio}
			<ChatAttachmentsPreviewCurrentItemAudio {currentItem} {audioSrc} />
		{:else if isVideo}
			<ChatAttachmentsPreviewCurrentItemVideo {currentItem} {videoSrc} />
		{:else if isUnavailable}
			<ChatAttachmentsPreviewCurrentItemUnavailable {IconComponent} />
		{/if}
	{/key}
{/if}
