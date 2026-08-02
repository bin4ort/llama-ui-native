#ifndef SERVER_H
#define SERVER_H

#define BACKEND_PORT   8765
#define FRONTEND_DIR   "frontend/v2"
#define BACKEND_URL    "http://localhost:8080"

#define VERSION        "0.3.0"
#define BUILD          "0x07D1E"  /* hex build number — update each build */

int server_start(void);
void server_stop(void);

#endif
