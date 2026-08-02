<script>import { useAttachmentMenu } from '$lib/hooks/use-attachment-menu.svelte';
let { class: className = '', disabled = false, hasAudioModality = false, hasVideoModality = false, hasVisionModality = false, hasMcpPromptsSupport = false, hasMcpResourcesSupport = false, onFileUpload, onSystemPromptClick, onMcpPromptClick, onMcpSettingsClick, onMcpResourcesClick } = $props();
let dropdownOpen = $state(false);
function handleMcpSettingsClick() {
    dropdownOpen = false;
    onMcpSettingsClick?.();
}
const attachmentMenu = useAttachmentMenu(() => ({
    hasVisionModality,
    hasAudioModality,
    hasVideoModality,
    hasMcpPromptsSupport,
    hasMcpResourcesSupport
}), () => ({ onFileUpload, onSystemPromptClick, onMcpPromptClick, onMcpResourcesClick }), () => {
    dropdownOpen = false;
});
</script>

<div class="flex items-center gap-1 {className}">
	<DropdownMenu.Root bind:open={dropdownOpen}>
		<!-- ignoreNonKeyboardFocus prevents the tooltip from flashing when the
		     menu closes and focus returns to the trigger -->
		<Tooltip.Root ignoreNonKeyboardFocus>
			<Tooltip.Trigger>
				{#snippet child({ props })}
					<DropdownMenu.Trigger
						{...props}
						class={cn(
							buttonVariants({ variant: 'secondary' }),
							'file-upload-button h-8 w-8 cursor-pointer rounded-full p-0'
						)}
						{disabled}
					>
						<span class="sr-only">{ATTACHMENT_TOOLTIP_TEXT}</span>

						<Plus class={ICON_CLASS_DEFAULT} />
					</DropdownMenu.Trigger>
				{/snippet}
			</Tooltip.Trigger>

			<Tooltip.Content>
				<p>{ATTACHMENT_TOOLTIP_TEXT}</p>
			</Tooltip.Content>
		</Tooltip.Root>

		<DropdownMenu.Content align="start" class="w-52">
			<ChatFormActionAddReasoningSubmenu />

			<DropdownMenu.Separator />

			<DropdownMenu.Sub>
				<DropdownMenu.SubTrigger class="flex cursor-pointer items-center gap-2">
					<File class={ICON_CLASS_DEFAULT} />

					<span>{t('Add files')}</span>
				</DropdownMenu.SubTrigger>

				<DropdownMenu.SubContent class="w-48">
					{#each ATTACHMENT_FILE_ITEMS as item (item.id)}
						{@const enabled = attachmentMenu.isItemEnabled(item.enabledWhen)}
						{#if enabled}
							<DropdownMenu.Item
								class="{item.class ?? ''} flex cursor-pointer items-center gap-2"
								onclick={() => attachmentMenu.callbacks[item.action]()}
							>
								<item.icon class={ICON_CLASS_DEFAULT} />

								<span>{item.label}</span>
							</DropdownMenu.Item>
						{:else if item.disabledTooltip}
							<Tooltip.Root delayDuration={TOOLTIP_DELAY_DURATION}>
								<Tooltip.Trigger tabindex={-1}>
									{#snippet child({ props })}
										<div {...props} class="cursor-default">
											<DropdownMenu.Item
												class="{item.class ?? ''} flex items-center gap-2"
												disabled
											>
												<item.icon class={ICON_CLASS_DEFAULT} />

												<span>{item.label}</span>
											</DropdownMenu.Item>
										</div>
									{/snippet}
								</Tooltip.Trigger>

								<Tooltip.Content side="right">
									<p>{item.disabledTooltip}</p>
								</Tooltip.Content>
							</Tooltip.Root>
						{/if}
					{/each}
				</DropdownMenu.SubContent>
			</DropdownMenu.Sub>

			<DropdownMenu.Item
				class="flex cursor-pointer items-center gap-2"
				onclick={onSystemPromptClick}
			>
				<MessageSquare class={ICON_CLASS_DEFAULT} />

				<span>{t('System Message')}</span>
			</DropdownMenu.Item>

			<ChatFormActionAddToolsSubmenu />

			<ChatFormActionAddMcpServersSubmenu onMcpSettingsClick={handleMcpSettingsClick} />

			{#if hasMcpPromptsSupport}
				<DropdownMenu.Separator />

				<DropdownMenu.Item
					class="flex cursor-pointer items-center gap-2"
					onclick={onMcpPromptClick}
				>
					<Zap class={ICON_CLASS_DEFAULT} />

					<span>{t('MCP Prompt')}</span>
				</DropdownMenu.Item>
			{/if}

			{#if hasMcpResourcesSupport}
				<DropdownMenu.Item
					class="flex cursor-pointer items-center gap-2"
					onclick={onMcpResourcesClick}
				>
					<FolderOpen class={ICON_CLASS_DEFAULT} />

					<span>{t('MCP Resources')}</span>
				</DropdownMenu.Item>
			{/if}
		</DropdownMenu.Content>
	</DropdownMenu.Root>
</div>
