/*
 * MIT License — Copyright (c) 2025 Llama UI Native
 * See LICENSE file in the project root.
 */
#ifndef SERVER_H
#define SERVER_H

#define BACKEND_PORT   8765
#define FRONTEND_DIR   "frontend/v2"
#define BACKEND_URL    "http://localhost:8080"

/* Keep VERSION/BUILD in sync with src/lib/constants/app.ts and package.json */
#define VERSION        "0.4.0"
#define BUILD          "0x07D1F"  /* hex build number — bump each release */

int server_start(void);
void server_stop(void);

#endif
