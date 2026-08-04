# LLMUI Error Codes

| Code | Area | Message | Status |
|---|---|---|---|
| LLMUI-SYS-000 | SYS | boot: entry script failed | active |
| LLMUI-SYS-001 | SYS | boot: kernel initialization failed | active |
| LLMUI-SYS-002 | SYS | router: no route handler matched | active |
| LLMUI-SYS-003 | SYS | router: hash parse failed | active |
| LLMUI-CFG-000 | CFG | settings: localStorage read/parse failed | active |
| LLMUI-CFG-001 | CFG | settings: localStorage write failed | active |
| LLMUI-CFG-002 | CFG | settings: server prop sync failed | active |
| LLMUI-CFG-003 | CFG | settings: reset to defaults failed | active |
| LLMUI-CFG-004 | CFG | settings: import rejected (bad payload) | active |
| LLMUI-DB-000 | DB | dexie: database open failed | active |
| LLMUI-DB-001 | DB | dexie: conversation add failed | active |
| LLMUI-DB-002 | DB | dexie: message add failed | active |
| LLMUI-DB-003 | DB | dexie: update failed | active |
| LLMUI-DB-004 | DB | dexie: delete failed | active |
| LLMUI-DB-005 | DB | dexie: conversation tree build failed | active |
| LLMUI-DB-006 | DB | migration: legacy key migration failed | active |
| LLMUI-DB-007 | DB | export: conversations serialization failed | active |
| LLMUI-DB-008 | DB | import: conversations parse failed | active |
| LLMUI-API-000 | API | http: request failed (network) | active |
| LLMUI-API-001 | API | http: non-2xx response | active |
| LLMUI-API-002 | API | api: response parse failed | active |
| LLMUI-API-003 | API | api: models list fetch failed | active |
| LLMUI-API-004 | API | api: props fetch failed | active |
| LLMUI-API-005 | API | api: slots fetch failed | active |
| LLMUI-API-006 | API | api: tools list (/tools) fetch failed | active |
| LLMUI-API-007 | API | api: tool execute failed | active |
| LLMUI-API-008 | API | api: model load/unload failed | active |
| LLMUI-API-009 | API | api: unauthorized (401/403) | active |
| LLMUI-API-010 | API | cors-proxy: upstream request failed | active |
| LLMUI-API-011 | API | http: response exceeded size cap | active |
| LLMUI-API-012 | API | fetch_url: unsupported protocol | active |
| LLMUI-STR-000 | STR | stream: connection failed | active |
| LLMUI-STR-001 | STR | stream: SSE frame parse failed | active |
| LLMUI-STR-002 | STR | stream: aborted mid-chunk | active |
| LLMUI-STR-003 | STR | stream: resume failed | active |
| LLMUI-STR-004 | STR | stream: timeout | active |
| LLMUI-STR-005 | STR | stream: server error event | active |
| LLMUI-STR-006 | STR | stream: empty completion | active |
| LLMUI-STR-007 | STR | stream: tool-call chunk aggregation failed | active |
| LLMUI-STR-008 | STR | stream: completion id missing | active |
| LLMUI-STR-009 | STR | stream: timings parse failed | active |
| LLMUI-STR-010 | STR | stream: queue overflow (pending messages) | active |
| LLMUI-CHAT-000 | CHAT | chat: message tree insert failed | active |
| LLMUI-CHAT-001 | CHAT | chat: branch/regenerate failed | active |
| LLMUI-CHAT-002 | CHAT | chat: title generation failed | active |
| LLMUI-CHAT-003 | CHAT | chat: system prompt apply failed | active |
| LLMUI-CHAT-004 | CHAT | chat: pending message conflict | active |
| LLMUI-CHAT-005 | CHAT | chat: conversation delete failed | active |
| LLMUI-CHAT-006 | CHAT | chat: edit message failed | active |
| LLMUI-CHAT-007 | CHAT | chat: pre-encode failed | active |
| LLMUI-CHAT-008 | CHAT | chat: fork conversation failed | active |
| LLMUI-MD-000 | MD | markdown: marked parse failed | active |
| LLMUI-MD-001 | MD | markdown: latex render failed | active |
| LLMUI-MD-002 | MD | markdown: mermaid render failed | active |
| LLMUI-MD-003 | MD | markdown: svg block sanitize rejected | active |
| LLMUI-MD-004 | MD | markdown: dompurify sanitize failed | active |
| LLMUI-MD-005 | MD | markdown: table restorer failed | active |
| LLMUI-IMG-000 | IMG | image: decode failed | active |
| LLMUI-IMG-001 | IMG | image: resize failed | active |
| LLMUI-IMG-002 | IMG | image: EXIF orientation failed | active |
| LLMUI-IMG-003 | IMG | image: HEIC decode failed | active |
| LLMUI-AUD-000 | AUD | audio: mic access denied | active |
| LLMUI-AUD-001 | AUD | audio: recording failed | active |
| LLMUI-AUD-002 | AUD | audio: file decode failed | active |
| LLMUI-AUD-003 | AUD | audio: transcription failed | active |
| LLMUI-PDF-000 | PDF | pdf: worker load failed | active |
| LLMUI-PDF-001 | PDF | pdf: document parse failed | active |
| LLMUI-PDF-002 | PDF | pdf: text extraction failed | active |
| LLMUI-MCP-000 | MCP | mcp: connect failed | active |
| LLMUI-MCP-001 | MCP | mcp: initialize handshake failed | active |
| LLMUI-MCP-002 | MCP | mcp: tools list failed | active |
| LLMUI-MCP-003 | MCP | mcp: resources list failed | active |
| LLMUI-MCP-004 | MCP | mcp: prompts list failed | active |
| LLMUI-MCP-005 | MCP | mcp: tool call failed | active |
| LLMUI-MCP-006 | MCP | mcp: resource read failed | active |
| LLMUI-MCP-007 | MCP | mcp: prompt get failed | active |
| LLMUI-TL-000 | TL | tools: agentic loop exceeded max turns | active |
| LLMUI-TL-001 | TL | tools: tool call parse failed | active |
| LLMUI-TL-002 | TL | tools: permission decision missing | active |
| LLMUI-TL-003 | TL | tools: verification cancelled/failed | active |
| LLMUI-TL-004 | TL | tools: built-in executor failed | active |
| LLMUI-TL-005 | TL | tools: sandbox execution failed | active |
| LLMUI-TL-006 | TL | tools: steering message inject failed | active |
| LLMUI-PRS-000 | PRS | presets: library parse failed | active |
| LLMUI-PRS-001 | PRS | presets: wizard generation failed | active |
| LLMUI-PRS-002 | PRS | presets: wizard JSON parse failed | active |
| LLMUI-PRS-003 | PRS | presets: change_preset target not found | active |
| LLMUI-PRS-004 | PRS | presets: seeding marker write failed | active |
| LLMUI-SEC-000 | SEC | sandbox: harness build failed | active |
| LLMUI-SEC-001 | SEC | sandbox: worker timeout | active |
| LLMUI-SEC-002 | SEC | sandbox: output cap exceeded | active |
| LLMUI-SEC-003 | SEC | redact: sensitive value leak detected | active |
| LLMUI-PWA-000 | PWA | sw: install/activate failed | active |
| LLMUI-PWA-001 | PWA | sw: precache failed | active |
| LLMUI-PWA-002 | PWA | sw: update check failed | active |
| LLMUI-UI-000 | UI | ui: modal open failed | active |
| LLMUI-UI-001 | UI | ui: toast failed | active |
| LLMUI-UI-002 | UI | ui: render error boundary | active |
| LLMUI-UI-003 | UI | ui: clipboard api failed | active |

## Native C server (SRV area)

| Code | Area | Message | Status |
|---|---|---|---|
| LLMUI-SRV-000 | SRV | server: start failed (socket bind/listen) | active |
| LLMUI-SRV-001 | SRV | server: static file not found | active |
| LLMUI-SRV-002 | SRV | server: static file read failed | active |
| LLMUI-SRV-003 | SRV | proxy: upstream DNS/connect failed | active |
| LLMUI-SRV-004 | SRV | proxy: upstream TLS/HTTP error | active |
| LLMUI-SRV-005 | SRV | proxy: upstream timed out | active |
| LLMUI-SRV-006 | SRV | proxy: upstream returned non-2xx | active |
| LLMUI-SRV-007 | SRV | proxy: request body too large | active |
| LLMUI-SRV-008 | SRV | api: malformed JSON body | active |
| LLMUI-SRV-009 | SRV | api: unauthorized (missing/invalid api key) | active |
| LLMUI-SRV-010 | SRV | api: method not allowed | active |
| LLMUI-SRV-011 | SRV | api: unknown endpoint | active |
| LLMUI-SRV-012 | SRV | api: cors-proxy target blocked | active |
| LLMUI-SRV-013 | SRV | server: network-info probe failed | active |
| LLMUI-SRV-014 | SRV | server: internal error | active |
| LLMUI-SRV-015 | SRV | server: health degraded | active |
