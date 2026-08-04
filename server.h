/*
 * Llama UI Native — Copyright (C) 2025 Llama UI Native contributors
 * Licensed under the GNU General Public License v3 or later — see LICENSE.
 */
#ifndef SERVER_H
#define SERVER_H

#define BACKEND_PORT   8765
#define FRONTEND_DIR   "frontend/v2"
#define BACKEND_URL    "http://localhost:8080"

/* Keep VERSION/BUILD in sync with src/lib/constants/app.ts and package.json */
#define VERSION        "0.4.3"
#define BUILD          "0x07D21"  /* hex build number — bump each release */

int server_start(void);
void server_stop(void);

#endif
