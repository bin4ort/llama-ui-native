/**
 * mcpResourceStore - Reactive State Store for MCP Resources
 *
 * Manages MCP protocol resources:
 * - Resource discovery and listing per server
 * - Resource content caching
 * - Resource subscriptions
 * - Resource attachments for chat context
 *
 * @see MCP Protocol Specification: https://modelcontextprotocol.io/specification/2025-06-18/server/resources
 */
import { SvelteMap } from 'svelte/reactivity';
import { AttachmentType } from '$lib/enums';
import { MCP_RESOURCE_ATTACHMENT_ID_PREFIX, MCP_RESOURCE_CACHE_MAX_ENTRIES, MCP_RESOURCE_CACHE_TTL_MS, NEWLINE, RESOURCE_UNKNOWN_TYPE, BINARY_CONTENT_LABEL } from '$lib/constants';
import { normalizeResourceUri } from '$lib/utils';
function generateAttachmentId() {
    return `${MCP_RESOURCE_ATTACHMENT_ID_PREFIX}-${Date.now()}-${Math.random().toString(36).substring(2, 9)}`;
}
class MCPResourceStore {
    _serverResources = $state(new SvelteMap());
    _cachedResources = $state(new SvelteMap());
    _subscriptions = $state(new SvelteMap());
    _attachments = $state([]);
    _isLoading = $state(false);
    get serverResources() {
        return this._serverResources;
    }
    get cachedResources() {
        return this._cachedResources;
    }
    get subscriptions() {
        return this._subscriptions;
    }
    get attachments() {
        return this._attachments;
    }
    get isLoading() {
        return this._isLoading;
    }
    get totalResourceCount() {
        let count = 0;
        for (const serverRes of this._serverResources.values()) {
            count += serverRes.resources.length;
        }
        return count;
    }
    get totalTemplateCount() {
        let count = 0;
        for (const serverRes of this._serverResources.values()) {
            count += serverRes.templates.length;
        }
        return count;
    }
    get attachmentCount() {
        return this._attachments.length;
    }
    get hasAttachments() {
        return this._attachments.length > 0;
    }
    /**
     *
     *
     * Server Resources Management
     *
     *
     */
    /**
     * Set resources for a server (called after listResources)
     */
    setServerResources(serverName, resources, templates) {
        this._serverResources.set(serverName, {
            serverName,
            resources,
            templates,
            lastFetched: new Date(),
            loading: false,
            error: undefined
        });
        console.log(`[MCPResources][${serverName}] Set ${resources.length} resources, ${templates.length} templates`);
    }
    /**
     * Set loading state for a server's resources
     */
    setServerLoading(serverName, loading) {
        const existing = this._serverResources.get(serverName);
        if (existing) {
            this._serverResources.set(serverName, { ...existing, loading });
        }
        else {
            this._serverResources.set(serverName, {
                serverName,
                resources: [],
                templates: [],
                loading,
                error: undefined
            });
        }
    }
    /**
     * Set error state for a server's resources
     */
    setServerError(serverName, error) {
        const existing = this._serverResources.get(serverName);
        if (existing) {
            this._serverResources.set(serverName, { ...existing, loading: false, error });
        }
        else {
            this._serverResources.set(serverName, {
                serverName,
                resources: [],
                templates: [],
                loading: false,
                error
            });
        }
    }
    /**
     * Get resources for a specific server
     */
    getServerResources(serverName) {
        return this._serverResources.get(serverName);
    }
    /**
     * Get all resources as MCPResourceInfo array (flattened with server names)
     */
    getAllResourceInfos() {
        const result = [];
        for (const [serverName, serverRes] of this._serverResources) {
            for (const resource of serverRes.resources) {
                result.push({
                    uri: resource.uri,
                    name: resource.name,
                    title: resource.title,
                    description: resource.description,
                    mimeType: resource.mimeType,
                    serverName,
                    annotations: resource.annotations,
                    icons: resource.icons
                });
            }
        }
        return result;
    }
    /**
     * Get all templates as MCPResourceTemplateInfo array (flattened with server names)
     */
    getAllTemplateInfos() {
        const result = [];
        for (const [serverName, serverRes] of this._serverResources) {
            for (const template of serverRes.templates) {
                result.push({
                    uriTemplate: template.uriTemplate,
                    name: template.name,
                    title: template.title,
                    description: template.description,
                    mimeType: template.mimeType,
                    serverName,
                    annotations: template.annotations,
                    icons: template.icons
                });
            }
        }
        return result;
    }
    /**
     * Clear resources for a server (e.g., when disconnected)
     */
    clearServerResources(serverName) {
        this._serverResources.delete(serverName);
        // Also clear cached content for this server's resources
        for (const [uri, cached] of this._cachedResources) {
            if (cached.resource.serverName === serverName) {
                this._cachedResources.delete(uri);
            }
        }
        // Clear subscriptions for this server
        for (const [uri, sub] of this._subscriptions) {
            if (sub.serverName === serverName) {
                this._subscriptions.delete(uri);
            }
        }
        console.log(`[MCPResources][${serverName}] Cleared all resources`);
    }
    /**
     *
     *
     * Resource Content Caching
     *
     *
     */
    /**
     * Cache resource content after reading
     */
    cacheResourceContent(resource, content) {
        // Enforce cache size limit
        if (this._cachedResources.size >= MCP_RESOURCE_CACHE_MAX_ENTRIES) {
            // Remove oldest entry
            const oldestKey = this._cachedResources.keys().next().value;
            if (oldestKey) {
                this._cachedResources.delete(oldestKey);
            }
        }
        this._cachedResources.set(resource.uri, {
            resource,
            content,
            fetchedAt: new Date(),
            subscribed: this._subscriptions.has(resource.uri)
        });
        console.log(`[MCPResources] Cached content for: ${resource.uri}`);
    }
    /**
     * Get cached content for a resource
     */
    getCachedContent(uri) {
        const cached = this._cachedResources.get(uri);
        if (!cached)
            return undefined;
        // Check if cache is still valid
        const age = Date.now() - cached.fetchedAt.getTime();
        if (age > MCP_RESOURCE_CACHE_TTL_MS && !cached.subscribed) {
            // Cache expired and not subscribed, remove it
            this._cachedResources.delete(uri);
            return undefined;
        }
        return cached;
    }
    /**
     * Invalidate cached content for a resource (e.g., on update notification)
     */
    invalidateCache(uri) {
        this._cachedResources.delete(uri);
        console.log(`[MCPResources] Invalidated cache for: ${uri}`);
    }
    /**
     * Clear all cached content
     */
    clearCache() {
        this._cachedResources.clear();
        console.log(`[MCPResources] Cleared all cached content`);
    }
    /**
     *
     *
     * Subscriptions
     *
     *
     */
    /**
     * Register a subscription for a resource
     */
    addSubscription(uri, serverName) {
        this._subscriptions.set(uri, {
            uri,
            serverName,
            subscribedAt: new Date()
        });
        // Update cached resource if exists
        const cached = this._cachedResources.get(uri);
        if (cached) {
            this._cachedResources.set(uri, { ...cached, subscribed: true });
        }
        console.log(`[MCPResources] Added subscription: ${uri}`);
    }
    /**
     * Remove a subscription for a resource
     */
    removeSubscription(uri) {
        this._subscriptions.delete(uri);
        // Update cached resource if exists
        const cached = this._cachedResources.get(uri);
        if (cached) {
            this._cachedResources.set(uri, { ...cached, subscribed: false });
        }
        console.log(`[MCPResources] Removed subscription: ${uri}`);
    }
    /**
     * Check if a resource is subscribed
     */
    isSubscribed(uri) {
        return this._subscriptions.has(uri);
    }
    /**
     * Handle resource update notification
     */
    handleResourceUpdate(uri) {
        // Invalidate cache so next read gets fresh content
        this.invalidateCache(uri);
        // Update subscription last update time
        const sub = this._subscriptions.get(uri);
        if (sub) {
            this._subscriptions.set(uri, { ...sub, lastUpdate: new Date() });
        }
        console.log(`[MCPResources] Resource updated: ${uri}`);
    }
    /**
     * Handle resources list changed notification
     */
    handleResourcesListChanged(serverName) {
        // Mark server resources as needing refresh
        const existing = this._serverResources.get(serverName);
        if (existing) {
            this._serverResources.set(serverName, {
                ...existing,
                lastFetched: undefined // Mark as stale
            });
        }
        console.log(`[MCPResources][${serverName}] Resources list changed, needs refresh`);
    }
    /**
     *
     *
     * Attachments (for chat context)
     *
     *
     */
    /**
     * Add a resource attachment to the current chat context
     */
    addAttachment(resource) {
        const attachment = {
            id: generateAttachmentId(),
            resource,
            loading: true
        };
        this._attachments = [...this._attachments, attachment];
        console.log(`[MCPResources] Added attachment: ${resource.uri}`);
        return attachment;
    }
    /**
     * Update attachment with fetched content
     */
    updateAttachmentContent(attachmentId, content) {
        this._attachments = this._attachments.map((att) => att.id === attachmentId ? { ...att, content, loading: false, error: undefined } : att);
    }
    /**
     * Update attachment with error
     */
    updateAttachmentError(attachmentId, error) {
        this._attachments = this._attachments.map((att) => att.id === attachmentId ? { ...att, loading: false, error } : att);
    }
    /**
     * Remove an attachment
     */
    removeAttachment(attachmentId) {
        this._attachments = this._attachments.filter((att) => att.id !== attachmentId);
        console.log(`[MCPResources] Removed attachment: ${attachmentId}`);
    }
    /**
     * Clear all attachments
     */
    clearAttachments() {
        this._attachments = [];
        console.log(`[MCPResources] Cleared all attachments`);
    }
    /**
     * Get attachment by ID
     */
    getAttachment(attachmentId) {
        return this._attachments.find((att) => att.id === attachmentId);
    }
    /**
     * Check if a resource is already attached
     */
    isAttached(uri) {
        const normalizedUri = normalizeResourceUri(uri);
        return this._attachments.some((att) => att.resource.uri === uri || normalizeResourceUri(att.resource.uri) === normalizedUri);
    }
    /**
     *
     *
     * Utility Methods
     *
     *
     */
    /**
     * Set global loading state
     */
    setLoading(loading) {
        this._isLoading = loading;
    }
    /**
     * Find resource info by URI across all servers
     */
    findResourceByUri(uri) {
        const normalizedUri = normalizeResourceUri(uri);
        for (const [serverName, serverRes] of this._serverResources) {
            const resource = serverRes.resources.find((r) => r.uri === uri) ??
                serverRes.resources.find((r) => normalizeResourceUri(r.uri) === normalizedUri);
            if (resource) {
                return {
                    uri: resource.uri,
                    name: resource.name,
                    title: resource.title,
                    description: resource.description,
                    mimeType: resource.mimeType,
                    serverName,
                    annotations: resource.annotations,
                    icons: resource.icons
                };
            }
        }
        return undefined;
    }
    /**
     * Find server name for a resource URI
     */
    findServerForUri(uri) {
        for (const [serverName, serverRes] of this._serverResources) {
            if (serverRes.resources.some((r) => r.uri === uri)) {
                return serverName;
            }
        }
        return undefined;
    }
    /**
     * Clear all state (e.g., on full reset)
     */
    clear() {
        this._serverResources.clear();
        this._cachedResources.clear();
        this._subscriptions.clear();
        this._attachments = [];
        this._isLoading = false;
        console.log(`[MCPResources] Cleared all state`);
    }
    /**
     * Get resource content as text for chat context
     * Formats content for inclusion in LLM prompts
     */
    formatAttachmentsForContext() {
        if (this._attachments.length === 0)
            return '';
        const parts = [];
        for (const attachment of this._attachments) {
            if (attachment.error)
                continue;
            if (!attachment.content || attachment.content.length === 0)
                continue;
            const resourceName = attachment.resource.title || attachment.resource.name;
            const serverName = attachment.resource.serverName;
            for (const content of attachment.content) {
                if ('text' in content && content.text) {
                    parts.push(`\n\n--- Resource: ${resourceName} (from ${serverName}) ---\n${content.text}`);
                }
                else if ('blob' in content && content.blob) {
                    // For binary content, just note it exists
                    parts.push(`\n\n--- Resource: ${resourceName} (from ${serverName}) ---\n[${BINARY_CONTENT_LABEL}: ${content.mimeType || RESOURCE_UNKNOWN_TYPE}]`);
                }
            }
        }
        return parts.join('');
    }
    /**
     * Convert current resource attachments to DatabaseMessageExtra[] for persisting with a message.
     * Each attachment becomes a DatabaseMessageExtraMcpResource stored on the user message.
     */
    toMessageExtras() {
        const extras = [];
        for (const attachment of this._attachments) {
            if (attachment.error)
                continue;
            if (!attachment.content || attachment.content.length === 0)
                continue;
            const resourceName = attachment.resource.title || attachment.resource.name;
            const contentParts = [];
            for (const content of attachment.content) {
                if ('text' in content && content.text) {
                    contentParts.push(content.text);
                }
                else if ('blob' in content && content.blob) {
                    contentParts.push(`[${BINARY_CONTENT_LABEL}: ${content.mimeType || RESOURCE_UNKNOWN_TYPE}]`);
                }
            }
            if (contentParts.length > 0) {
                extras.push({
                    type: AttachmentType.MCP_RESOURCE,
                    name: resourceName,
                    uri: attachment.resource.uri,
                    serverName: attachment.resource.serverName,
                    content: contentParts.join(NEWLINE),
                    mimeType: attachment.resource.mimeType
                });
            }
        }
        return extras;
    }
}
export const mcpResourceStore = new MCPResourceStore();
// Export convenience functions
export const mcpResources = () => mcpResourceStore.serverResources;
export const mcpResourceAttachments = () => mcpResourceStore.attachments;
export const mcpResourceAttachmentCount = () => mcpResourceStore.attachmentCount;
export const mcpHasResourceAttachments = () => mcpResourceStore.hasAttachments;
export const mcpTotalResourceCount = () => mcpResourceStore.totalResourceCount;
export const mcpResourcesLoading = () => mcpResourceStore.isLoading;
