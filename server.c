/*
 * Llama UI Native — Copyright (C) 2025 Llama UI Native contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * See LICENSE file in the project root.
 *
 * Minimal embedded HTTP server (thread-per-connection):
 *   - static file serving from FRONTEND_DIR (SPA fallback to index.html)
 *   - a few JSON endpoints
 *   - SSE proxy of /v1/chat/completions to the local llama.cpp backend
 */
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>

static pthread_t g_thread;
static volatile int g_running = 0;
static volatile int g_listen_fd = -1;

/* Max request header size (method line + headers). */
#define MAX_HEADER 16384
/* Max accepted POST body (attachments can be large). */
#define MAX_BODY   (64L * 1024 * 1024)

/* ---- helpers ---- */

static size_t send_all(int fd, const void *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, (const char *)data + sent, len - sent, MSG_NOSIGNAL);
        if (n <= 0)
            break;
        sent += (size_t)n;
    }
    return sent;
}

static const void *find_bytes(const void *hay, size_t hlen, const void *needle,
    size_t nlen) {
    if (nlen == 0 || hlen < nlen)
        return NULL;
    const unsigned char *h = hay;
    for (size_t i = 0; i + nlen <= hlen; i++) {
        if (memcmp(h + i, needle, nlen) == 0)
            return h + i;
    }
    return NULL;
}

static void send_headers(int fd, int status, const char *text,
    const char *ctype, long clen, int cache_asset) {
    char buf[640];
    int n = snprintf(buf, sizeof(buf),
        "HTTP/1.1 %d %s\r\n"
        "Connection: close\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Cache-Control: %s\r\n"
        "Content-Type: %s\r\n",
        status, text,
        cache_asset ? "public, max-age=31536000, immutable" : "no-cache",
        ctype);
    if (clen >= 0)
        n += snprintf(buf + n, sizeof(buf) - n, "Content-Length: %ld\r\n", clen);
    n += snprintf(buf + n, sizeof(buf) - n, "\r\n");
    send_all(fd, buf, (size_t)n);
}

static void send_json(int fd, const char *json) {
    send_headers(fd, 200, "OK", "application/json", (long)strlen(json), 0);
    send_all(fd, json, strlen(json));
}

/* URL-decode %XX sequences in place. Returns 0 if the input was invalid. */
static int url_decode(char *s) {
    char *w = s;
    for (char *r = s; *r; r++) {
        if (*r == '%' && r[1] && r[2]) {
            int hi = r[1] >= '0' && r[1] <= '9' ? r[1] - '0'
                : (r[1] & ~0x20) - 'A' + 10;
            int lo = r[2] >= '0' && r[2] <= '9' ? r[2] - '0'
                : (r[2] & ~0x20) - 'A' + 10;
            if ((hi < 0 || hi > 15) || (lo < 0 || lo > 15))
                return 0;
            char c = (char)(hi * 16 + lo);
            if (c == 0)
                return 0; /* embedded NUL — reject */
            *w++ = c;
            r += 2;
            continue;
        }
        *w++ = *r;
    }
    *w = 0;
    return 1;
}

static int parse_request(const char *req, char *method, size_t mcap,
    char *path, size_t pcap, long *clen) {
    const char *p = req;
    size_t i = 0;
    while (*p && *p != ' ' && *p != '\r' && i < mcap - 1)
        method[i++] = *p++;
    method[i] = 0;
    if (*p != ' ')
        return 0;
    p++;
    i = 0;
    while (*p && *p != ' ' && *p != '\r' && i < pcap - 1)
        path[i++] = *p++;
    path[i] = 0;
    if (i == 0 || path[0] != '/')
        return 0;

    /* strip query string and URL-decode the path */
    char *q = strchr(path, '?');
    if (q)
        *q = 0;
    if (!url_decode(path))
        return 0;

    /* scan headers for Content-Length */
    *clen = 0;
    const char *hdr = strstr(req, "\r\n");
    if (hdr) {
        hdr += 2;
        const char *end = strstr(hdr, "\r\n");
        while (end && end > hdr) {
            if (strncasecmp(hdr, "content-length:", 15) == 0)
                *clen = strtol(hdr + 15, NULL, 10);
            hdr = end + 2;
            end = strstr(hdr, "\r\n");
        }
    }
    if (*clen < 0)
        *clen = 0;
    return 1;
}

static const char *mime_for(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot)
        return "application/octet-stream";
    if (strcasecmp(dot, ".html") == 0 || strcasecmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcasecmp(dot, ".js") == 0 || strcasecmp(dot, ".mjs") == 0)
        return "application/javascript";
    if (strcasecmp(dot, ".css") == 0)
        return "text/css; charset=utf-8";
    if (strcasecmp(dot, ".json") == 0 || strcasecmp(dot, ".map") == 0)
        return "application/json";
    if (strcasecmp(dot, ".svg") == 0)
        return "image/svg+xml";
    if (strcasecmp(dot, ".png") == 0)
        return "image/png";
    if (strcasecmp(dot, ".ico") == 0)
        return "image/x-icon";
    if (strcasecmp(dot, ".woff2") == 0)
        return "font/woff2";
    if (strcasecmp(dot, ".jxl") == 0)
        return "image/jxl";
    if (strcasecmp(dot, ".webmanifest") == 0)
        return "application/manifest+json";
    if (strcasecmp(dot, ".txt") == 0)
        return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

static void send_file(int fd, const char *path, const struct stat *st,
    int cache_asset) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        const char *msg = "Not Found";
        send_headers(fd, 404, "Not Found", "text/plain", (long)strlen(msg), 0);
        send_all(fd, msg, strlen(msg));
        return;
    }
    size_t size = st->st_size > 0 ? (size_t)st->st_size : 0;
    char *data = malloc(size ? size : 1);
    if (!data) {
        fclose(f);
        const char *msg = "Internal Server Error";
        send_headers(fd, 500, "Internal Server Error", "text/plain",
            (long)strlen(msg), 0);
        send_all(fd, msg, strlen(msg));
        return;
    }
    size_t got = fread(data, 1, size, f);
    fclose(f);
    send_headers(fd, 200, "OK", mime_for(path), (long)got, cache_asset);
    send_all(fd, data, got);
    free(data);
}

static void serve_static(int fd, const char *path) {
    if (strstr(path, "..")) {
        const char *msg = "Not Found";
        send_headers(fd, 404, "Not Found", "text/plain", (long)strlen(msg), 0);
        send_all(fd, msg, strlen(msg));
        return;
    }

    char real[4096];
    if (strcmp(path, "/") == 0)
        snprintf(real, sizeof(real), "%s/index.html", FRONTEND_DIR);
    else
        snprintf(real, sizeof(real), "%s%s", FRONTEND_DIR, path);

    struct stat st;
    if (stat(real, &st) == 0 && S_ISDIR(st.st_mode))
        snprintf(real, sizeof(real), "%s%s/index.html", FRONTEND_DIR, path);

    if (stat(real, &st) != 0 || !S_ISREG(st.st_mode)) {
        /* SPA fallback: unknown route without a file extension → index.html */
        const char *slash = strrchr(path, '/');
        const char *last = slash ? slash + 1 : path;
        if (!strchr(last, '.')) {
            snprintf(real, sizeof(real), "%s/index.html", FRONTEND_DIR);
            if (stat(real, &st) == 0) {
                send_file(fd, real, &st, 0);
                return;
            }
        }
        const char *msg = "Not Found";
        send_headers(fd, 404, "Not Found", "text/plain", (long)strlen(msg), 0);
        send_all(fd, msg, strlen(msg));
        return;
    }

    send_file(fd, real, &st, strncmp(path, "/_app/", 6) == 0);
}

/* ---- endpoints ---- */

static void handle_network_info(int fd) {
    char ip[64] = "127.0.0.1";
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s >= 0) {
        struct sockaddr_in sa = {0};
        sa.sin_family = AF_INET;
        sa.sin_port = htons(80);
        inet_pton(AF_INET, "8.8.8.8", &sa.sin_addr);
        if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) == 0) {
            struct sockaddr_in l = {0};
            socklen_t len = sizeof(l);
            getsockname(s, (struct sockaddr *)&l, &len);
            inet_ntop(AF_INET, &l.sin_addr, ip, sizeof(ip));
        }
        close(s);
    }

    char j[256];
    snprintf(j, sizeof(j),
        "{\"ip\":\"%s\",\"port\":%d,\"url\":\"http://%s:%d\"}",
        ip, BACKEND_PORT, ip, BACKEND_PORT);
    send_json(fd, j);
}

/* ---- SSE proxy to llama.cpp backend ---- */

struct proxy_ctx {
    int fd;
};

static size_t proxy_write_cb(void *ptr, size_t sz, size_t n, void *d) {
    struct proxy_ctx *ctx = d;
    size_t total = sz * n;
    if (total == 0)
        return 0;
    if (send_all(ctx->fd, ptr, total) != total)
        return 0; /* abort the transfer on write error */
    return total;
}

static void handle_proxy(int fd, const char *body, size_t blen) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        send_json(fd, "{\"error\":\"curl_init_failed\"}");
        return;
    }

    char url[512];
    snprintf(url, sizeof(url), "%s/v1/chat/completions", BACKEND_URL);

    struct curl_slist *h =
        curl_slist_append(NULL, "Content-Type: application/json");

    struct proxy_ctx ctx = {.fd = fd};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (body && blen > 0) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)blen);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, proxy_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    send_headers(fd, 200, "OK", "text/event-stream", -1, 0);

    CURLcode r = curl_easy_perform(curl);
    if (r != CURLE_OK && r != CURLE_WRITE_ERROR) {
        char err[256];
        snprintf(err, sizeof(err), "data:{\"error\":\"%s\"}\n\n",
            curl_easy_strerror(r));
        send_all(fd, err, strlen(err));
    }

    curl_slist_free_all(h);
    curl_easy_cleanup(curl);
}

/* ---- connection handling (one thread per connection) ---- */

static void handle_connection(int fd) {
    struct timeval tv = {.tv_sec = 15, .tv_usec = 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    char buf[MAX_HEADER + 65536];
    size_t n = 0;
    while (n < sizeof(buf)) {
        ssize_t r = recv(fd, buf + n, sizeof(buf) - n, 0);
        if (r <= 0)
            return;
        n += (size_t)r;
        if (find_bytes(buf, n, "\r\n\r\n", 4))
            break;
    }

    const void *hdr_end_p = find_bytes(buf, n, "\r\n\r\n", 4);
    if (!hdr_end_p)
        return; /* header too large or malformed */
    size_t hlen = (const char *)hdr_end_p - buf + 4;

    char req[MAX_HEADER + 1];
    size_t req_len = hlen < sizeof(req) ? hlen : sizeof(req);
    memcpy(req, buf, req_len);
    req[req_len] = 0;

    char method[8], path[4096];
    long clen = 0;
    if (!parse_request(req, method, sizeof(method), path, sizeof(path), &clen)) {
        const char *msg = "Bad Request";
        send_headers(fd, 400, "Bad Request", "text/plain", (long)strlen(msg), 0);
        send_all(fd, msg, strlen(msg));
        return;
    }

    /* gather body */
    char *body = NULL;
    size_t have = n - hlen;
    if (clen > 0) {
        if (clen > MAX_BODY)
            return;
        body = malloc((size_t)clen);
        if (!body)
            return;
        size_t got = have < (size_t)clen ? have : (size_t)clen;
        if (got)
            memcpy(body, buf + hlen, got);
        while (got < (size_t)clen) {
            ssize_t r = recv(fd, body + got, (size_t)clen - got, 0);
            if (r <= 0) {
                free(body);
                return;
            }
            got += (size_t)r;
        }
    }

    if (strcmp(path, "/health") == 0 || strcmp(path, "/v1/health") == 0 ||
        strcmp(path, "/api/config") == 0) {
        char j[160];
        snprintf(j, sizeof(j),
            "{\"status\":\"ok\",\"version\":\"%s\",\"build\":\"%s\"}",
            VERSION, BUILD);
        send_json(fd, j);
    } else if (strcmp(path, "/api/network-info") == 0) {
        handle_network_info(fd);
    } else if (strcmp(path, "/v1/models") == 0) {
        send_json(fd,
            "{\"object\":\"list\",\"data\":[{\"id\":\"local-model\","
            "\"object\":\"model\",\"owned_by\":\"local\"}]}");
    } else if (strcmp(path, "/props") == 0) {
        send_json(fd,
            "{\"default_generation_settings\":{"
            "\"n_ctx\":4096,\"n_predict\":-1,"
            "\"params\":{"
            "\"temperature\":0.8,\"top_k\":40,\"top_p\":0.95,"
            "\"min_p\":0.05,\"typ_p\":1.0,"
            "\"xtc_probability\":0.0,\"xtc_threshold\":0.1,"
            "\"dynatemp_range\":0.0,\"dynatemp_exponent\":1.0,"
            "\"max_tokens\":-1,\"samplers\":[\"top_k\",\"typ_p\","
            "\"top_p\",\"min_p\",\"temperature\"],"
            "\"repeat_last_n\":64,\"repeat_penalty\":1.0,"
            "\"presence_penalty\":0.0,\"frequency_penalty\":0.0,"
            "\"dry_multiplier\":0.0,\"dry_base\":1.75,"
            "\"dry_allowed_length\":2,\"dry_penalty_last_n\":-1"
            "}},"
            "\"total_slots\":1,\"model\":\"local-model\"}");
    } else if (strcmp(path, "/slots") == 0) {
        send_json(fd,
            "[{\"id\":0,\"state\":0,"
            "\"model\":\"local-model\"}]");
    } else if (strcmp(path, "/v2") == 0) {
        static const char rsp[] =
            "HTTP/1.1 307 Temporary Redirect\r\n"
            "Connection: close\r\n"
            "Location: /v2/\r\n"
            "Content-Length: 0\r\n\r\n";
        send_all(fd, rsp, strlen(rsp));
    } else if (strcmp(path, "/v1/chat/completions") == 0 ||
        strcmp(path, "/completion") == 0) {
        handle_proxy(fd, body, clen > 0 ? (size_t)clen : 0);
    } else if (strcmp(method, "GET") == 0) {
        serve_static(fd, path);
    } else {
        send_json(fd, "{\"error\":\"method not allowed\"}");
    }

    free(body);
}

static void *conn_thread(void *arg) {
    int fd = (int)(intptr_t)arg;
    handle_connection(fd);
    close(fd);
    return NULL;
}

/* ---- server thread ---- */

static void *server_thread(void *arg) {
    (void)arg;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return NULL;
    int one = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_ANY);
    a.sin_port = htons(BACKEND_PORT);
    if (bind(fd, (struct sockaddr *)&a, sizeof(a)) != 0 ||
        listen(fd, 16) != 0) {
        close(fd);
        return NULL;
    }

    g_listen_fd = fd;
    g_running = 1;
    while (g_running) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int c = accept(fd, (struct sockaddr *)&caddr, &clen);
        if (c < 0) {
            if (!g_running)
                break;
            usleep(10000);
            continue;
        }
        pthread_t t;
        if (pthread_create(&t, NULL, conn_thread, (void *)(intptr_t)c) != 0)
            close(c);
        else
            pthread_detach(t);
    }

    close(fd);
    g_listen_fd = -1;
    return NULL;
}

int server_start(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    if (pthread_create(&g_thread, NULL, server_thread, NULL) != 0) {
        curl_global_cleanup();
        return -1;
    }
    for (int i = 0; i < 30; i++) {
        if (g_running)
            return 0;
        usleep(500000);
    }
    return -1;
}

void server_stop(void) {
    g_running = 0;
    int fd = g_listen_fd;
    if (fd >= 0)
        shutdown(fd, SHUT_RDWR); /* unblock accept() */
    pthread_join(g_thread, NULL);
    curl_global_cleanup();
}
