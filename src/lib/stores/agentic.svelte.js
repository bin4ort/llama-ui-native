/**
 * agenticStore - Reactive State Store for Agentic Loop Orchestration
 *
 * Manages multi-turn agentic loop with MCP tools:
 * - LLM streaming with tool call detection
 * - Tool execution via mcpStore
 * - Session state management
 * - Turn limit enforcement
 *
 * Each agentic turn produces separate DB messages:
 * - One assistant message per LLM turn (with tool_calls if any)
 * - One tool result message per tool call execution
 *
 * **Architecture & Relationships:**
 * - **ChatService**: Stateless API layer (sendMessage, streaming)
 * - **mcpStore**: MCP connection management and tool execution
 * - **agenticStore** (this): Reactive state + business logic
 *
 * @see ChatService in services/chat.service.ts for API operations
 * @see mcpStore in stores/mcp.svelte.ts for MCP operations
 */
import { ChatService } from '$lib/services';
import { config } from '$lib/stores/settings.svelte';
import { mcpStore } from '$lib/stores/mcp.svelte';
import { modelsStore } from '$lib/stores/models.svelte';
import { toolsStore } from '$lib/stores/tools.svelte';
import { permissionsStore } from '$lib/stores/permissions.svelte';
import { BuiltInTool, ToolSource, ToolPermissionDecision } from '$lib/enums';
import { SvelteMap } from 'svelte/reactivity';
import { ToolsService } from '$lib/services/tools.service';
import { SandboxService } from '$lib/services/sandbox.service';
import { isAbortError } from '$lib/utils';
import { DEFAULT_AGENTIC_CONFIG, NEWLINE } from '$lib/constants';
import { IMAGE_MIME_TO_EXTENSION, DATA_URI_BASE64_REGEX, MCP_ATTACHMENT_NAME_PREFIX, DEFAULT_IMAGE_EXTENSION } from '$lib/constants';
import { AttachmentType, ContentPartType, MessageRole, MimeTypePrefix, ToolCallType } from '$lib/enums';
function createDefaultSession() {
    return {
        isRunning: false,
        currentTurn: 0,
        totalToolCalls: 0,
        lastError: null,
        streamingToolCall: null,
        pendingPermissionRequest: null,
        executingToolCallId: null
    };
}
function toAgenticMessages(messages) {
    return messages.map((message) => {
        if (message.role === MessageRole.ASSISTANT &&
            message.tool_calls &&
            message.tool_calls.length > 0) {
            return {
                role: MessageRole.ASSISTANT,
                content: message.content,
                reasoning_content: message.reasoning_content,
                tool_calls: message.tool_calls.map((call, index) => ({
                    id: call.id ?? `call_${index}`,
                    type: call.type ?? ToolCallType.FUNCTION,
                    function: {
                        name: call.function?.name ?? '',
                        arguments: call.function?.arguments ?? ''
                    }
                }))
            };
        }
        if (message.role === MessageRole.ASSISTANT) {
            return {
                role: MessageRole.ASSISTANT,
                content: message.content,
                reasoning_content: message.reasoning_content
            };
        }
        if (message.role === MessageRole.TOOL && message.tool_call_id) {
            return {
                role: MessageRole.TOOL,
                tool_call_id: message.tool_call_id,
                content: typeof message.content === 'string' ? message.content : ''
            };
        }
        return {
            role: message.role,
            content: message.content
        };
    });
}
class AgenticStore {
    _sessions = new SvelteMap();
    /** Dedicated reactive state for pending permission requests (ensures immediate UI updates) */
    _pendingPermissions = new SvelteMap();
    /** Non-reactive: stores resolve functions for pending permission Promises */
    _permissionResolvers = new Map();
    /** Dedicated reactive state for pending continue requests (turn limit reached) */
    _pendingContinueRequests = new SvelteMap();
    /** Non-reactive: stores resolve functions for pending continue Promises */
    _continueResolvers = new Map();
    /** Reactive: queued steering messages to inject between turns */
    _steeringMessages = new SvelteMap();
    get isReady() {
        return true;
    }
    get isAnyRunning() {
        for (const session of this._sessions.values()) {
            if (session.isRunning)
                return true;
        }
        return false;
    }
    getSession(conversationId) {
        let session = this._sessions.get(conversationId);
        if (!session) {
            session = createDefaultSession();
            this._sessions.set(conversationId, session);
        }
        return session;
    }
    updateSession(conversationId, update) {
        const session = this.getSession(conversationId);
        this._sessions.set(conversationId, { ...session, ...update });
    }
    clearSession(conversationId) {
        this._sessions.delete(conversationId);
    }
    getActiveSessions() {
        const active = [];
        for (const [conversationId, session] of this._sessions.entries()) {
            if (session.isRunning)
                active.push({ conversationId, session });
        }
        return active;
    }
    isRunning(conversationId) {
        return this._sessions.get(conversationId)?.isRunning ?? false;
    }
    currentTurn(conversationId) {
        return this._sessions.get(conversationId)?.currentTurn ?? 0;
    }
    totalToolCalls(conversationId) {
        return this._sessions.get(conversationId)?.totalToolCalls ?? 0;
    }
    lastError(conversationId) {
        return this._sessions.get(conversationId)?.lastError ?? null;
    }
    streamingToolCall(conversationId) {
        return this._sessions.get(conversationId)?.streamingToolCall ?? null;
    }
    executingToolCallId(conversationId) {
        return this._sessions.get(conversationId)?.executingToolCallId ?? null;
    }
    pendingPermissionRequest(conversationId) {
        return this._pendingPermissions.get(conversationId) ?? null;
    }
    pendingContinueRequest(conversationId) {
        return this._pendingContinueRequests.get(conversationId) ?? false;
    }
    resolveContinue(conversationId, shouldContinue) {
        const resolver = this._continueResolvers.get(conversationId);
        if (resolver) {
            this._continueResolvers.delete(conversationId);
            resolver(shouldContinue);
        }
    }
    resolvePermission(conversationId, decision) {
        const resolver = this._permissionResolvers.get(conversationId);
        if (resolver) {
            this._permissionResolvers.delete(conversationId);
            resolver(decision);
        }
    }
    clearError(conversationId) {
        this.updateSession(conversationId, { lastError: null });
    }
    hasPendingSteeringMessage(conversationId) {
        return this._steeringMessages.has(conversationId);
    }
    pendingSteeringMessageContent(conversationId) {
        return this._steeringMessages.get(conversationId)?.content ?? null;
    }
    pendingSteeringMessageExtras(conversationId) {
        return this._steeringMessages.get(conversationId)?.extras;
    }
    /**
     * Queue a steering message. When the current agentic turn completes,
     * the flow exits and the caller re-sends the messagenormal chat message.
     */
    injectSteeringMessage(conversationId, content, extras) {
        this._steeringMessages.set(conversationId, { content, extras });
    }
    /**
     * Clear the pending steering message without consuming it.
     */
    clearSteeringMessage(conversationId) {
        this._steeringMessages.delete(conversationId);
    }
    /**
     * Consume and return the pending steering message for re-sending.
     * Called by chatStore after the agentic flow exits.
     */
    consumePendingSteeringMessage(conversationId) {
        const msg = this._steeringMessages.get(conversationId);
        if (!msg)
            return null;
        this._steeringMessages.delete(conversationId);
        return msg;
    }
    getConfig(settings, perChatOverrides) {
        const maxTurns = Number(settings.agenticMaxTurns) || DEFAULT_AGENTIC_CONFIG.maxTurns;
        const hasTools = mcpStore.hasEnabledServers(perChatOverrides) ||
            toolsStore.builtinTools.length > 0 ||
            toolsStore.frontendTools.length > 0 ||
            toolsStore.customTools.length > 0;
        return {
            enabled: hasTools && DEFAULT_AGENTIC_CONFIG.enabled,
            maxTurns
        };
    }
    parseToolArguments(args) {
        if (typeof args === 'object')
            return args;
        const trimmed = args.trim();
        if (trimmed === '')
            return {};
        return JSON.parse(trimmed);
    }
    async requestPermission(conversationId, toolName, serverLabel, signal) {
        const permissionKey = toolsStore.getPermissionKey(toolName);
        if (permissionKey && permissionsStore.hasTool(permissionKey)) {
            return ToolPermissionDecision.ONCE;
        }
        this._pendingPermissions.set(conversationId, { toolName, serverLabel });
        return new Promise((resolve) => {
            if (signal?.aborted) {
                this._pendingPermissions.set(conversationId, null);
                resolve(ToolPermissionDecision.DENY);
                return;
            }
            this._permissionResolvers.set(conversationId, (decision) => {
                this._pendingPermissions.set(conversationId, null);
                if (decision === ToolPermissionDecision.ALWAYS && permissionKey) {
                    permissionsStore.allowTool(permissionKey);
                }
                else if (decision === ToolPermissionDecision.ALWAYS_SERVER) {
                    const serverToolKeys = toolsStore.allTools
                        .filter((t) => t.serverName
                        ? t.serverName === serverLabel
                        : toolsStore.getToolServerLabel(t.definition.function.name) === serverLabel)
                        .map((t) => toolsStore.getPermissionKey(t.definition.function.name))
                        .filter((k) => k !== null);
                    permissionsStore.allowTools(serverToolKeys);
                }
                resolve(decision);
            });
            signal?.addEventListener('abort', () => {
                const resolver = this._permissionResolvers.get(conversationId);
                if (resolver) {
                    this._permissionResolvers.delete(conversationId);
                    this._pendingPermissions.set(conversationId, null);
                    resolve(ToolPermissionDecision.DENY);
                }
            }, { once: true });
        });
    }
    async requestContinue(conversationId, signal) {
        this._pendingContinueRequests.set(conversationId, true);
        return new Promise((resolve) => {
            if (signal?.aborted) {
                this._pendingContinueRequests.set(conversationId, false);
                resolve(false);
                return;
            }
            this._continueResolvers.set(conversationId, (shouldContinue) => {
                this._pendingContinueRequests.set(conversationId, false);
                resolve(shouldContinue);
            });
            signal?.addEventListener('abort', () => {
                const resolver = this._continueResolvers.get(conversationId);
                if (resolver) {
                    this._continueResolvers.delete(conversationId);
                    this._pendingContinueRequests.set(conversationId, false);
                    resolve(false);
                }
            }, { once: true });
        });
    }
    async runAgenticFlow(params) {
        const { conversationId, messages, options = {}, callbacks, signal, perChatOverrides } = params;
        // Clear any pending permissions/continue requests for this conversation when starting a new flow
        this._pendingPermissions.set(conversationId, null);
        this._permissionResolvers.delete(conversationId);
        this._pendingContinueRequests.set(conversationId, false);
        this._continueResolvers.delete(conversationId);
        this._steeringMessages.delete(conversationId);
        // Ensure built-in tools are fetched before checking if agentic is enabled
        if (toolsStore.builtinTools.length === 0 && !toolsStore.loading) {
            await toolsStore.fetchBuiltinTools();
        }
        const agenticConfig = this.getConfig(config(), perChatOverrides);
        if (!agenticConfig.enabled)
            return { handled: false };
        const hasMcpServers = mcpStore.hasEnabledServers(perChatOverrides);
        if (hasMcpServers) {
            const initialized = await mcpStore.ensureInitialized(perChatOverrides);
            if (!initialized) {
                console.log('[AgenticStore] MCP not initialized');
            }
        }
        const tools = toolsStore.getEnabledToolsForLLM();
        if (tools.length === 0) {
            return { handled: false };
        }
        console.log(`[AgenticStore] Starting agentic flow with ${tools.length} tools`);
        const normalizedMessages = (await Promise.all(messages.map((msg) => {
            if ('id' in msg && 'convId' in msg && 'timestamp' in msg)
                return ChatService.convertDbMessageToApiChatMessageData(msg);
            return msg;
        }))).filter((msg) => {
            if (msg.role === MessageRole.SYSTEM) {
                const content = typeof msg.content === 'string' ? msg.content : '';
                return content.trim().length > 0;
            }
            return true;
        });
        this.updateSession(conversationId, {
            isRunning: true,
            currentTurn: 0,
            totalToolCalls: 0,
            lastError: null
        });
        if (hasMcpServers)
            mcpStore.acquireConnection();
        try {
            await this.executeAgenticLoop({
                conversationId,
                messages: normalizedMessages,
                options,
                tools,
                agenticConfig,
                callbacks,
                signal
            });
            return { handled: true };
        }
        catch (error) {
            const normalizedError = error instanceof Error ? error : new Error(String(error));
            this.updateSession(conversationId, { lastError: normalizedError });
            callbacks.onError?.(normalizedError);
            return { handled: true, error: normalizedError };
        }
        finally {
            this.updateSession(conversationId, { isRunning: false });
            if (hasMcpServers) {
                await mcpStore
                    .releaseConnection()
                    .catch((err) => console.warn('[AgenticStore] Failed to release MCP connection:', err));
            }
        }
    }
    async executeAgenticLoop(params) {
        const { conversationId, messages, options, tools, agenticConfig, callbacks, signal } = params;
        const { onChunk, onReasoningChunk, onToolCallsStreaming, onAttachments, onModel, onCompletionId, onAssistantTurnComplete, createToolResultMessage, updateToolResultMessage, createAssistantMessage, onFlowComplete, onTimings, onTurnComplete } = callbacks;
        const sessionMessages = toAgenticMessages(messages);
        let capturedTimings;
        let totalToolCallCount = 0;
        const agenticTimings = {
            turns: 0,
            toolCallsCount: 0,
            toolsMs: 0,
            toolCalls: [],
            perTurn: [],
            llm: { predicted_n: 0, predicted_ms: 0, prompt_n: 0, prompt_ms: 0 }
        };
        const maxTurns = agenticConfig.maxTurns;
        const effectiveModel = options.model || modelsStore.models[0]?.model || '';
        let turn = 0;
        while (true) {
            if (turn >= maxTurns) {
                // Turn limit reached - ask user whether to continue
                const shouldContinue = await this.requestContinue(conversationId, signal);
                // Yield to allow Svelte to flush the UI update
                await new Promise((r) => setTimeout(r, 0));
                if (!shouldContinue || signal?.aborted) {
                    onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                    return;
                }
                // User chose to continue - extend the limit
                turn = 0;
            }
            this.updateSession(conversationId, { currentTurn: turn + 1 });
            agenticTimings.turns = turn + 1;
            if (signal?.aborted) {
                onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                return;
            }
            // For turns > 0, create a new assistant message via callback
            if (turn > 0 && createAssistantMessage) {
                await createAssistantMessage();
            }
            let turnContent = '';
            let turnReasoningContent = '';
            let turnToolCalls = [];
            let lastStreamingToolCallName = '';
            let lastStreamingToolCallArgsLength = 0;
            let turnTimings;
            const turnStats = {
                turn: turn + 1,
                llm: { predicted_n: 0, predicted_ms: 0, prompt_n: 0, prompt_ms: 0 },
                toolCalls: [],
                toolsMs: 0
            };
            try {
                await ChatService.sendMessage(sessionMessages, {
                    ...options,
                    stream: true,
                    tools: tools.length > 0 ? tools : undefined,
                    onChunk: (chunk) => {
                        turnContent += chunk;
                        onChunk?.(chunk);
                    },
                    onReasoningChunk: (chunk) => {
                        turnReasoningContent += chunk;
                        onReasoningChunk?.(chunk);
                    },
                    onToolCallChunk: (serialized) => {
                        try {
                            turnToolCalls = JSON.parse(serialized);
                            onToolCallsStreaming?.(turnToolCalls);
                            if (turnToolCalls.length > 0 && turnToolCalls[0]?.function) {
                                const name = turnToolCalls[0].function.name || '';
                                const args = turnToolCalls[0].function.arguments || '';
                                const argsLengthBucket = Math.floor(args.length / 100);
                                if (name !== lastStreamingToolCallName ||
                                    argsLengthBucket !== lastStreamingToolCallArgsLength) {
                                    lastStreamingToolCallName = name;
                                    lastStreamingToolCallArgsLength = argsLengthBucket;
                                    this.updateSession(conversationId, {
                                        streamingToolCall: { name, arguments: args }
                                    });
                                }
                            }
                        }
                        catch {
                            /* Ignore parse errors during streaming */
                        }
                    },
                    onModel,
                    onCompletionId,
                    onTimings: (timings, progress) => {
                        onTimings?.(timings, progress);
                        if (timings) {
                            capturedTimings = timings;
                            turnTimings = timings;
                        }
                    },
                    onComplete: () => {
                        /* Completion handled after sendMessage resolves */
                    },
                    onError: (error) => {
                        throw error;
                    }
                }, conversationId, signal);
                this.updateSession(conversationId, { streamingToolCall: null });
                if (turnTimings) {
                    agenticTimings.llm.predicted_n += turnTimings.predicted_n || 0;
                    agenticTimings.llm.predicted_ms += turnTimings.predicted_ms || 0;
                    agenticTimings.llm.prompt_n += turnTimings.prompt_n || 0;
                    agenticTimings.llm.prompt_ms += turnTimings.prompt_ms || 0;
                    turnStats.llm.predicted_n = turnTimings.predicted_n || 0;
                    turnStats.llm.predicted_ms = turnTimings.predicted_ms || 0;
                    turnStats.llm.prompt_n = turnTimings.prompt_n || 0;
                    turnStats.llm.prompt_ms = turnTimings.prompt_ms || 0;
                }
            }
            catch (error) {
                if (signal?.aborted) {
                    // Save whatever we have for this turn before exiting
                    await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, this.buildFinalTimings(capturedTimings, agenticTimings), undefined);
                    onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                    return;
                }
                const normalizedError = error instanceof Error ? error : new Error('LLM stream error');
                // preserve partial output as is, the outer error dialog informs the user separately
                await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, this.buildFinalTimings(capturedTimings, agenticTimings), undefined);
                onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                throw normalizedError;
            }
            // If the abort landed while ChatService.sendMessage was still resolving, the
            // outer catch above never fires because ChatService swallows the AbortError
            // and returns normally. Bail out here so a half-received tool_call (truncated
            // arguments JSON) is not persisted as if it were complete.
            if (signal?.aborted) {
                await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, this.buildFinalTimings(capturedTimings, agenticTimings), undefined);
                onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                return;
            }
            // === Steering check: if a user message was queued during this turn, exit the flow.
            // The caller (chatStore) will consume the pending message and re-send it normally.
            if (this._steeringMessages.has(conversationId)) {
                console.log('[AgenticStore] Steering message detected after turn, exiting agentic flow');
                await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, this.buildFinalTimings(capturedTimings, agenticTimings), turnToolCalls.length > 0 ? this.normalizeToolCalls(turnToolCalls) : undefined);
                onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                return;
            }
            // No tool calls = final turn, save and complete
            if (turnToolCalls.length === 0) {
                agenticTimings.perTurn.push(turnStats);
                const finalTimings = this.buildFinalTimings(capturedTimings, agenticTimings);
                await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, finalTimings, undefined);
                if (finalTimings)
                    onTurnComplete?.(finalTimings);
                onFlowComplete?.(finalTimings);
                return;
            }
            // Normalize and save assistant turn with tool calls
            const normalizedCalls = this.normalizeToolCalls(turnToolCalls);
            if (normalizedCalls.length === 0) {
                await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, this.buildFinalTimings(capturedTimings, agenticTimings), undefined);
                onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                return;
            }
            totalToolCallCount += normalizedCalls.length;
            this.updateSession(conversationId, { totalToolCalls: totalToolCallCount });
            // Save the assistant message with its tool calls
            await onAssistantTurnComplete?.(turnContent, turnReasoningContent || undefined, turnTimings, normalizedCalls);
            // Add assistant message to session history
            sessionMessages.push({
                role: MessageRole.ASSISTANT,
                content: turnContent || undefined,
                reasoning_content: turnReasoningContent || undefined,
                tool_calls: normalizedCalls
            });
            // Execute each tool call and create result messages
            for (let i = 0; i < normalizedCalls.length; i++) {
                const toolCall = normalizedCalls[i];
                if (signal?.aborted) {
                    onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                    return;
                }
                // Check for pending steering message - skip remaining tool calls
                if (this._steeringMessages.has(conversationId)) {
                    console.log(`[AgenticStore] Steering message detected, skipping ${normalizedCalls.length - i} remaining tool call(s)`);
                    for (let j = i; j < normalizedCalls.length; j++) {
                        const remainingCall = normalizedCalls[j];
                        const interruptedContent = 'Tool execution was interrupted by a new user message.';
                        if (createToolResultMessage) {
                            await createToolResultMessage(remainingCall.id, interruptedContent);
                        }
                        sessionMessages.push({
                            role: MessageRole.TOOL,
                            tool_call_id: remainingCall.id,
                            content: interruptedContent
                        });
                    }
                    break;
                }
                const toolName = toolCall.function.name;
                const serverLabel = toolsStore.getToolServerLabel(toolName);
                // Ask for permission before executing the tool
                const permission = await this.requestPermission(conversationId, toolName, serverLabel, signal);
                // Yield to allow Svelte to flush the UI update (hide permission dialog)
                await new Promise((r) => setTimeout(r, 0));
                if (signal?.aborted) {
                    onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                    return;
                }
                const toolStartTime = performance.now();
                const toolSource = toolsStore.getToolSource(toolName);
                let result = '';
                let toolSuccess = true;
                let createdToolResultMessageId = null;
                // Streaming tools (currently only exec_shell_command): mark
                // the session so the matching renderer can switch to live mode.
                // Cleared unconditionally below.
                this.updateSession(conversationId, { executingToolCallId: toolCall.id });
                if (permission === ToolPermissionDecision.DENY) {
                    result = 'Tool execution was denied by the user.';
                    toolSuccess = false;
                }
                else {
                    try {
                        if (toolSource === ToolSource.BUILTIN &&
                            toolName === BuiltInTool.EXEC_SHELL_COMMAND &&
                            createToolResultMessage &&
                            updateToolResultMessage) {
                            const args = this.parseToolArguments(toolCall.function.arguments);
                            const msg = await createToolResultMessage(toolCall.id, '');
                            createdToolResultMessageId = msg.id;
                            let accumulated = '';
                            for await (const ev of ToolsService.streamTool(toolName, args, signal)) {
                                if (ev.chunk !== null) {
                                    accumulated += ev.chunk;
                                    await updateToolResultMessage(msg.id, accumulated);
                                }
                                if (ev.done) {
                                    if (ev.error) {
                                        accumulated = accumulated
                                            ? `${accumulated}\nError: ${ev.error}`
                                            : `Error: ${ev.error}`;
                                        await updateToolResultMessage(msg.id, accumulated);
                                        toolSuccess = false;
                                    }
                                    break;
                                }
                            }
                            result = accumulated;
                        }
                        else if (toolSource === ToolSource.BUILTIN) {
                            const args = this.parseToolArguments(toolCall.function.arguments);
                            const executionResult = await ToolsService.executeTool(toolName, args, signal);
                            result = executionResult.content;
                            if (executionResult.isError)
                                toolSuccess = false;
                        }
                        else if (toolSource === ToolSource.FRONTEND) {
                            const args = this.parseToolArguments(toolCall.function.arguments);
                            const executionResult = await SandboxService.executeTool(toolName, args, signal);
                            result = executionResult.content;
                            if (executionResult.isError)
                                toolSuccess = false;
                        }
                        else {
                            const mcpCall = {
                                id: toolCall.id,
                                function: { name: toolName, arguments: toolCall.function.arguments }
                            };
                            const executionResult = await mcpStore.executeTool(mcpCall, signal);
                            result = executionResult.content;
                        }
                    }
                    catch (error) {
                        if (isAbortError(error)) {
                            this.updateSession(conversationId, { executingToolCallId: null });
                            onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                            return;
                        }
                        // Carry the partial stream contents already mirrored to the UI -
                        // they show up as live output even if the stream broke off mid-run.
                        result = result
                            ? `${result}\nError: ${error instanceof Error ? error.message : String(error)}`
                            : `Error: ${error instanceof Error ? error.message : String(error)}`;
                        toolSuccess = false;
                        if (createdToolResultMessageId && updateToolResultMessage) {
                            await updateToolResultMessage(createdToolResultMessageId, result);
                        }
                    }
                }
                this.updateSession(conversationId, { executingToolCallId: null });
                const toolDurationMs = performance.now() - toolStartTime;
                const toolTiming = {
                    name: toolCall.function.name,
                    duration_ms: Math.round(toolDurationMs),
                    success: toolSuccess
                };
                agenticTimings.toolCalls.push(toolTiming);
                agenticTimings.toolCallsCount++;
                agenticTimings.toolsMs += Math.round(toolDurationMs);
                turnStats.toolCalls.push(toolTiming);
                turnStats.toolsMs += Math.round(toolDurationMs);
                if (signal?.aborted) {
                    onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                    return;
                }
                const { cleanedResult, attachments } = this.extractBase64Attachments(result);
                // For streaming tools the result message was created empty
                // at the start of execution and updated in place as chunks
                // arrived via updateToolResultMessage. Skip the second
                // create call - just attach any base64 attachments found in
                // the final accumulator (rare, since chunks usually don't
                // carry image data URIs) and emit the attachments callback.
                let toolResultMessage;
                if (createdToolResultMessageId) {
                    toolResultMessage = { id: createdToolResultMessageId };
                    if (attachments.length > 0 && updateToolResultMessage) {
                        await updateToolResultMessage(createdToolResultMessageId, cleanedResult, attachments);
                    }
                }
                else if (createToolResultMessage) {
                    toolResultMessage = await createToolResultMessage(toolCall.id, cleanedResult, attachments.length > 0 ? attachments : undefined);
                }
                if (attachments.length > 0 && toolResultMessage) {
                    onAttachments?.(toolResultMessage.id, attachments);
                }
                // Build content parts for session history (including images for vision models)
                const contentParts = [
                    { type: ContentPartType.TEXT, text: cleanedResult }
                ];
                for (const attachment of attachments) {
                    if (attachment.type === AttachmentType.IMAGE) {
                        if (modelsStore.modelSupportsVision(effectiveModel)) {
                            contentParts.push({
                                type: ContentPartType.IMAGE_URL,
                                image_url: {
                                    url: attachment.base64Url
                                }
                            });
                        }
                        else {
                            console.info(`[AgenticStore] Skipping image attachment (model "${effectiveModel}" does not support vision)`);
                        }
                    }
                }
                sessionMessages.push({
                    role: MessageRole.TOOL,
                    tool_call_id: toolCall.id,
                    content: contentParts.length === 1 ? cleanedResult : contentParts
                });
            }
            if (turnStats.toolCalls.length > 0) {
                agenticTimings.perTurn.push(turnStats);
                const intermediateTimings = this.buildFinalTimings(capturedTimings, agenticTimings);
                if (intermediateTimings)
                    onTurnComplete?.(intermediateTimings);
            }
            // If tools were interrupted by a steering message, exit now instead of starting another LLM turn
            if (this._steeringMessages.has(conversationId)) {
                console.log('[AgenticStore] Steering message detected after tool execution, exiting agentic flow');
                onFlowComplete?.(this.buildFinalTimings(capturedTimings, agenticTimings));
                return;
            }
            turn++;
        }
    }
    buildFinalTimings(capturedTimings, agenticTimings) {
        if (agenticTimings.toolCallsCount === 0)
            return capturedTimings;
        return {
            predicted_n: capturedTimings?.predicted_n,
            predicted_ms: capturedTimings?.predicted_ms,
            prompt_n: capturedTimings?.prompt_n,
            prompt_ms: capturedTimings?.prompt_ms,
            cache_n: capturedTimings?.cache_n,
            agentic: agenticTimings
        };
    }
    normalizeToolCalls(toolCalls) {
        if (!toolCalls)
            return [];
        return toolCalls.map((call, index) => ({
            id: call?.id ?? `tool_${index}`,
            type: call?.type ?? ToolCallType.FUNCTION,
            function: {
                name: call?.function?.name ?? '',
                arguments: call?.function?.arguments ?? ''
            }
        }));
    }
    extractBase64Attachments(result) {
        if (!result.trim()) {
            return { cleanedResult: result, attachments: [] };
        }
        const lines = result.split(NEWLINE);
        const attachments = [];
        let attachmentIndex = 0;
        const cleanedLines = lines.map((line) => {
            const trimmedLine = line.trim();
            const match = trimmedLine.match(DATA_URI_BASE64_REGEX);
            if (!match) {
                return line;
            }
            const mimeType = match[1].toLowerCase();
            const base64Data = match[2];
            if (!base64Data) {
                return line;
            }
            attachmentIndex += 1;
            const name = this.buildAttachmentName(mimeType, attachmentIndex);
            if (mimeType.startsWith(MimeTypePrefix.IMAGE)) {
                attachments.push({ type: AttachmentType.IMAGE, name, base64Url: trimmedLine });
                return `[Attachment saved: ${name}]`;
            }
            return line;
        });
        return { cleanedResult: cleanedLines.join(NEWLINE), attachments };
    }
    buildAttachmentName(mimeType, index) {
        const extension = IMAGE_MIME_TO_EXTENSION[mimeType] ?? DEFAULT_IMAGE_EXTENSION;
        return `${MCP_ATTACHMENT_NAME_PREFIX}-${Date.now()}-${index}.${extension}`;
    }
}
export const agenticStore = new AgenticStore();
export function agenticIsRunning(conversationId) {
    return agenticStore.isRunning(conversationId);
}
export function agenticCurrentTurn(conversationId) {
    return agenticStore.currentTurn(conversationId);
}
export function agenticTotalToolCalls(conversationId) {
    return agenticStore.totalToolCalls(conversationId);
}
export function agenticLastError(conversationId) {
    return agenticStore.lastError(conversationId);
}
export function agenticStreamingToolCall(conversationId) {
    return agenticStore.streamingToolCall(conversationId);
}
export function agenticPendingPermissionRequest(conversationId) {
    return agenticStore.pendingPermissionRequest(conversationId);
}
export function agenticResolvePermission(conversationId, decision) {
    agenticStore.resolvePermission(conversationId, decision);
}
export function agenticPendingContinueRequest(conversationId) {
    return agenticStore.pendingContinueRequest(conversationId);
}
export function agenticResolveContinue(conversationId, shouldContinue) {
    agenticStore.resolveContinue(conversationId, shouldContinue);
}
export function agenticHasPendingSteeringMessage(conversationId) {
    return agenticStore.hasPendingSteeringMessage(conversationId);
}
export function agenticInjectSteeringMessage(conversationId, content, extras) {
    agenticStore.injectSteeringMessage(conversationId, content, extras);
}
export function agenticPendingSteeringMessageContent(conversationId) {
    return agenticStore.pendingSteeringMessageContent(conversationId);
}
export function agenticPendingSteeringMessageExtras(conversationId) {
    return agenticStore.pendingSteeringMessageExtras(conversationId);
}
export function agenticClearSteeringMessage(conversationId) {
    agenticStore.clearSteeringMessage(conversationId);
}
export function agenticIsAnyRunning() {
    return agenticStore.isAnyRunning;
}
export function agenticExecutingToolCallId(conversationId) {
    return agenticStore.executingToolCallId(conversationId);
}
