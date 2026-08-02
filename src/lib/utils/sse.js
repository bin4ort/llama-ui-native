import { SSE_DATA_PREFIX, SSE_DONE_MARKER, SSE_LINE_SEPARATOR, SSE_RECORD_SEPARATOR } from '$lib/constants';
export async function* parseSseJsonStream(response, signal) {
    const reader = response.body?.getReader();
    if (!reader)
        return;
    const decoder = new TextDecoder();
    let buffer = '';
    try {
        while (true) {
            if (signal?.aborted)
                return;
            const { done, value } = await reader.read();
            if (done)
                break;
            buffer += decoder.decode(value, { stream: true });
            const records = buffer.split(SSE_RECORD_SEPARATOR);
            buffer = records.pop() ?? '';
            for (const record of records) {
                if (!record)
                    continue;
                for (const line of record.split(SSE_LINE_SEPARATOR)) {
                    if (!line.startsWith(SSE_DATA_PREFIX))
                        continue;
                    const payload = line.slice(SSE_DATA_PREFIX.length).trim();
                    if (payload === SSE_DONE_MARKER)
                        return;
                    if (!payload)
                        continue;
                    try {
                        yield { data: JSON.parse(payload) };
                    }
                    catch {
                        // Skip silently per the function contract above.
                    }
                }
            }
        }
    }
    finally {
        try {
            reader.releaseLock();
        }
        catch (error) {
            console.error('[sse] failed to release reader lock:', error);
        }
    }
}
