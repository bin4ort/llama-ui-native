/*
 * error-codes.h — native C server error-code registry (LLMUI-SRV-*).
 * Mirrors the frontend registry in web/kernel/error-codes.js; kept in sync
 * manually. Append-only — never reuse or renumber.
 * Docs: docs/ERROR-CODES.md
 */
#ifndef LLMUI_ERROR_CODES_H
#define LLMUI_ERROR_CODES_H

enum {
	LLMUI_SRV_000 = 0, /* server: start failed (socket bind/listen)     */
	LLMUI_SRV_001,     /* server: static file not found                  */
	LLMUI_SRV_002,     /* server: static file read failed                */
	LLMUI_SRV_003,     /* proxy: upstream DNS/connect failed             */
	LLMUI_SRV_004,     /* proxy: upstream TLS/HTTP error                  */
	LLMUI_SRV_005,     /* proxy: upstream timed out                       */
	LLMUI_SRV_006,     /* proxy: upstream returned non-2xx                */
	LLMUI_SRV_007,     /* proxy: request body too large                   */
	LLMUI_SRV_008,     /* api: malformed JSON body                        */
	LLMUI_SRV_009,     /* api: unauthorized (missing/invalid api key)     */
	LLMUI_SRV_010,     /* api: method not allowed                         */
	LLMUI_SRV_011,     /* api: unknown endpoint                           */
	LLMUI_SRV_012,     /* api: cors-proxy target blocked                  */
	LLMUI_SRV_013,     /* server: network-info probe failed               */
	LLMUI_SRV_014,     /* server: internal error                           */
	LLMUI_SRV_015      /* server: health degraded                          */
};

#endif /* LLMUI_ERROR_CODES_H */
