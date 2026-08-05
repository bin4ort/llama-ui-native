/**
 * registry.js — Agent B: ported settings field registry (visual parity).
 * Data extracted from src/lib/constants/settings-registry.ts — labels and
 * help texts are rendered verbatim, so they must match the old UI 1:1.
 * Sections: sampling / penalties / general / display / agentic / developer
 * / tools / import-export. Types: input (number/text), checkbox, textarea.
 */
export const FIELDS = {
  sampling: [
    { key: 'temperature', label: 'Temperature', type: 'input', help: 'Controls the randomness of the generated text by affecting the probability distribution of the output tokens. Higher = more random, lower = more focused.' },
    { key: 'dynatemp_range', label: 'Dynamic temperature range', type: 'input', help: 'Addon for the temperature sampler. The added value to the range of dynamic temperature, which adjusts probabilities by entropy of tokens.' },
    { key: 'dynatemp_exponent', label: 'Dynamic temperature exponent', type: 'input', help: 'Addon for the temperature sampler. Smoothes out the probability redistribution based on the most probable token.' },
    { key: 'top_k', label: 'Top K', type: 'input', help: 'Keeps only k top tokens.' },
    { key: 'top_p', label: 'Top P', type: 'input', help: 'Limits tokens to those that together have a cumulative probability of p.' },
    { key: 'min_p', label: 'Min P', type: 'input', help: 'Limits tokens based on the minimum probability for a token to be considered.' },
    { key: 'xtc_probability', label: 'XTC probability', type: 'input', help: 'XTC sampler cuts out top tokens; this parameter controls the probability of applying the filter.' },
    { key: 'xtc_threshold', label: 'XTC threshold', type: 'input', help: 'XTC sampler cuts out top tokens; this parameter controls the minimum token probability to be considered.' },
    { key: 'typ_p', label: 'Typical P', type: 'input', help: 'Sorts and limits tokens based on the difference between log-probability and the average log-probability of the sample.' },
    { key: 'max_tokens', label: 'Max tokens', type: 'input', help: 'The maximum number of token per output. Use -1 for infinite.' },
    { key: 'samplers', label: 'Samplers', type: 'input', help: 'The order at which samplers are applied, in simplified way.' },
    { key: 'backend_sampling', label: 'Backend sampling', type: 'checkbox', help: 'Enable backend-based samplers. When enabled, supported samplers run on the server side instead of the frontend.' }
  ],
  penalties: [
    { key: 'repeat_last_n', label: 'Repeat last N', type: 'input', help: 'Last n tokens to consider for penalizing repetition' },
    { key: 'repeat_penalty', label: 'Repeat penalty', type: 'input', help: 'Controls the repetition of token sequences in the generated output.' },
    { key: 'presence_penalty', label: 'Presence penalty', type: 'input', help: 'Limits tokens based on whether they appear in the output or not.' },
    { key: 'frequency_penalty', label: 'Frequency penalty', type: 'input', help: 'Limits tokens based on how often they appear in the output.' },
    { key: 'dry_multiplier', label: 'DRY multiplier', type: 'input', help: 'DRY sampling reduces repetition in generated text even across paragraphs and contexts.' },
    { key: 'dry_base', label: 'DRY base', type: 'input', help: 'DRY sampling reduces repetition in generated text even across paragraphs and contexts.' },
    { key: 'dry_allowed_length', label: 'DRY allowed length', type: 'input', help: 'DRY sampling reduces repetition in generated text even across paragraphs and contexts.' },
    { key: 'dry_penalty_last_n', label: 'DRY penalty last N', type: 'input', help: 'DRY sampling reduces repetition in generated text even across paragraphs and contexts.' }
  ],
  general: [
    { key: 'language', label: 'Language', type: 'select', help: 'Choose the interface language.' },
    { key: 'theme', label: 'Theme', type: 'select', help: 'Choose the color theme for the interface.' },
    { key: 'serverEndpoint', label: 'Server Endpoint', type: 'input', help: 'Set the llama.cpp server or OpenAI-compatible API endpoint.' },
    { key: 'webUiAddress', label: 'Web UI Address', type: 'input', help: 'The address where the Web UI is hosted.' },
    { key: 'webUiEnabled', label: 'Enable Web UI', type: 'checkbox', help: 'Allow access to the Web UI from other devices.' },
    { key: 'apiKey', label: 'API Key', type: 'input', help: 'Optional API key for OpenAI-compatible endpoints.' },
    { key: 'systemMessage', label: 'Default system prompt', type: 'textarea', help: 'The default system prompt — the starting message that defines how the model behaves.' },
    { key: 'pasteLongTextToFileLen', label: 'Paste long text to file length', type: 'input', posInt: true, help: 'On pasting long text, it will be converted to a file. You can control the file length by setting the value of this parameter. Value 0 means disable.' },
    { key: 'sendOnEnter', label: 'Send message on Enter', type: 'checkbox', help: 'Use Enter to send messages and Shift + Enter for new lines. When disabled, use Ctrl/Cmd + Enter.' },
    { key: 'autoMicOnEmpty', label: 'Show microphone on empty input', type: 'checkbox', help: 'Automatically show microphone button instead of send button when textarea is empty for models with audio modality support.' },
    { key: 'enableContinueGeneration', label: 'Enable "Continue" button', type: 'checkbox', help: 'Enable "Continue" button for assistant messages, including reasoning models.' },
    { key: 'titleGenerationUseFirstLine', label: 'Conversation title', type: 'radio', help: 'Choose how conversation titles are generated.', options: [['true', 'Use first line'], ['false', 'Generate with LLM']] },
    { key: 'titleGenerationPrompt', label: 'LLM title generation prompt', type: 'textarea', help: 'Optional template for the title generation prompt. Use {{USER}} and {{ASSISTANT}} placeholders.' },
    { key: 'copyTextAttachmentsAsPlainText', label: 'Copy text attachments as plain text', type: 'checkbox', help: 'When copying a message with text attachments, combine them into a single plain text string instead of a special format that can be pasted back as attachments.' },
    { key: 'pdfAsImage', label: 'Parse PDF as image', type: 'checkbox', help: 'Parse PDF as image instead of text. Automatically falls back to text processing for non-vision models.' },
    { key: 'maxImageMPixels', label: 'Maximum image resolution (megapixels)', type: 'input', help: 'Images larger than this will be resized before sending to server. Set to 0 to disable.' }
  ],
  display: [
    { key: 'showMessageStats', label: 'Show message generation statistics', type: 'checkbox', help: 'Display generation statistics (tokens/second, token count, duration) for assistant messages.' },
    { key: 'showAgenticTurnStats', label: 'Show statistics for individual agentic turns', type: 'checkbox', help: 'Display per-turn statistics (tokens, duration) under each tool call.' },
    { key: 'showThoughtInProgress', label: 'Show thought in progress', type: 'checkbox', help: 'Expand thought process by default when generating messages.' },
    { key: 'alwaysShowToolCallContent', label: 'Always show tool call content', type: 'checkbox', help: 'Automatically expand tool call details while executing and keep them expanded afterwards.' },
    { key: 'renderUserContentAsMarkdown', label: 'Render user content as Markdown', type: 'checkbox', help: 'Render user messages using markdown formatting in the chat.' },
    { key: 'renderThinkingAsMarkdown', label: 'Render thinking as Markdown', type: 'checkbox', help: 'Render the reasoning/thinking block content as formatted Markdown.' },
    { key: 'fullHeightCodeBlocks', label: 'Use full height code blocks', type: 'checkbox', help: 'Always display code blocks at their full natural height, overriding the max-height limit.' },
    { key: 'disableAutoScroll', label: 'Disable automatic scroll', type: 'checkbox', help: 'Disable automatic scrolling while messages stream so you can read the output at your own pace.' },
    { key: 'alwaysShowSidebarOnDesktop', label: 'Always show sidebar on desktop', type: 'checkbox', help: 'Always keep the sidebar visible on desktop instead of auto-hiding it.' },
    { key: 'showRawModelNames', label: 'Show raw model names', type: 'checkbox', help: 'Display full raw model identifiers (e.g. "ggml-org/GLM-4.7-Flash-GGUF") instead of short labels.' },
    { key: 'showModelQuantization', label: 'Show model quantization information', type: 'checkbox', help: 'Display quantization badges (e.g. Q8_0, Q4_K_M) next to model names.' },
    { key: 'showModelTags', label: 'Show model tags', type: 'checkbox', help: 'Display model tags (e.g. "vision", "reasoning") next to model names.' },
    { key: 'customJson', label: 'Custom JSON', type: 'textarea', help: 'Custom JSON parameters to send to the API. Must be valid JSON.' },
    { key: 'customCss', label: 'Custom CSS', type: 'textarea', help: 'CSS injected into the page at runtime. Set it here, or ship it as a static file.' }
  ],
  agentic: [
    { key: 'agenticMaxTurns', label: 'Agentic turns', type: 'input', posInt: true, help: 'Maximum number of tool execution cycles before stopping (prevents infinite loops).' },
    { key: 'mcpRequestTimeoutSeconds', label: 'MCP request timeout (seconds)', type: 'input', posInt: true, help: 'Timeout for individual MCP tool calls.' }
  ],
  developer: [
    { key: 'preEncodeConversation', label: 'Pre-fill KV cache after response', type: 'checkbox', help: 'After each response, re-submit the conversation to pre-fill the server KV cache. Makes the next turn faster since the prompt is already encoded while you read the response.' },
    { key: 'disableReasoningParsing', label: 'Disable reasoning content parsing', type: 'checkbox', help: 'Send reasoning_format=none so the server returns thinking tokens inline instead of extracting them into a separate field.' },
    { key: 'excludeReasoningFromContext', label: 'Exclude reasoning from context', type: 'checkbox', help: 'Strip thinking from previous messages before sending. When off, thinking is sent back via the reasoning_content field so the model sees its own chain-of-thought across turns.' },
    { key: 'showRawOutputSwitch', label: 'Enable raw output toggle', type: 'checkbox', help: 'Show toggle button to display messages as plain text instead of Markdown-formatted content' },
    { key: 'jsSandboxEnabled', label: 'JavaScript sandbox tool', type: 'checkbox', help: 'Expose a run_javascript tool to the model. Code runs in a Web Worker sandbox.' },
    { key: 'symbolicMathEnabled', label: 'Symbolic math (nerdamer)', type: 'checkbox', help: 'Pre-load nerdamer in the sandbox for symbolic computation.' },
    { key: 'customJson', label: 'Custom JSON', type: 'textarea', help: 'Custom JSON parameters to send to the API. Must be valid JSON.' },
    { key: 'customCss', label: 'Custom CSS', type: 'textarea', help: 'CSS injected into the page at runtime. Set it here, or ship it as a static file.' }
  ],
  tools: [
    { key: 'toolCalculateEnabled', label: 'Calculator', type: 'checkbox', help: 'Built-in day-to-day tool: evaluate mathematical expressions.' },
    { key: 'toolFetchUrlEnabled', label: 'Fetch URL', type: 'checkbox', help: 'Built-in day-to-day tool: read a web page as text.' },
    { key: 'toolToTableEnabled', label: 'Table from data', type: 'checkbox', help: 'Built-in day-to-day tool: CSV/TSV/JSON into a markdown table.' },
    { key: 'toolJsonEnabled', label: 'JSON helper', type: 'checkbox', help: 'Built-in day-to-day tool: validate, format and query JSON.' },
    { key: 'toolClipboardEnabled', label: 'Clipboard', type: 'checkbox', help: 'Built-in day-to-day tool: read/write the system clipboard.' },
    { key: 'toolNotifyEnabled', label: 'Notifications', type: 'checkbox', help: 'Built-in day-to-day tool: desktop notifications.' },
    { key: 'toolTodoEnabled', label: 'Task list', type: 'checkbox', help: 'Built-in day-to-day tool: persistent per-conversation to-do list.' },
    { key: 'toolWeatherEnabled', label: 'Weather', type: 'checkbox', help: 'Built-in day-to-day tool: current conditions and forecast (Open-Meteo, no key).' },
    { key: 'toolWikipediaEnabled', label: 'Wikipedia lookup', type: 'checkbox', help: 'Built-in day-to-day tool: article summaries for factual grounding.' },
    { key: 'toolPlotChartEnabled', label: 'Chart', type: 'checkbox', help: 'Built-in day-to-day tool: line/bar charts from data (SVG).' }
  ]
};
