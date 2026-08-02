#include "server.h"
#define MG_ENABLE_LOG 0
#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>

static pthread_t g_thread;
static volatile int g_running = 0;

static void send_json(struct mg_connection *c, const char *json) {
    mg_http_reply(c, 200,
        "Access-Control-Allow-Origin:*\r\n"
        "Content-Type:application/json\r\n", "%s", json);
}

static int match_uri(struct mg_http_message *hm, const char *path) {
    return mg_match(hm->uri, mg_str(path), NULL);
}

/* ---- SSE proxy to llama.cpp backend ---- */
struct proxy_ctx {
    struct mg_connection *client;
};

static size_t proxy_write_cb(void *ptr, size_t sz, size_t n, void *d) {
    struct proxy_ctx *ctx = d;
    mg_send(ctx->client, ptr, sz * n);
    return sz * n;
}

static void handle_chat_proxy(struct mg_connection *c,
    struct mg_http_message *hm) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        send_json(c, "{\"error\":\"curl_init_failed\"}");
        return;
    }

    char url[512];
    snprintf(url, sizeof(url), "%s/v1/chat/completions", BACKEND_URL);

    char *body = NULL;
    size_t blen = hm->body.len;
    if (blen) {
        body = malloc(blen + 1);
        memcpy(body, hm->body.buf, blen);
        body[blen] = 0;
    }

    struct curl_slist *h =
        curl_slist_append(NULL, "Content-Type: application/json");

    struct proxy_ctx ctx = {.client = c};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (body) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)blen);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, proxy_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    mg_printf(c,
        "HTTP/1.1 200 OK\r\n"
        "Access-Control-Allow-Origin:*\r\n"
        "Content-Type:text/event-stream\r\n"
        "Cache-Control:no-cache\r\n"
        "Connection:keep-alive\r\n\r\n");

    CURLcode r = curl_easy_perform(curl);
    if (r != CURLE_OK && r != CURLE_WRITE_ERROR)
        mg_printf(c, "data:{\"error\":\"%s\"}\n\n", curl_easy_strerror(r));

    curl_slist_free_all(h);
    curl_easy_cleanup(curl);
    free(body);
}

/* ---- endpoint handlers ---- */
static void handle_health(struct mg_connection *c) {
    char buf[128];
    snprintf(buf, sizeof(buf),
        "{\"status\":\"ok\",\"version\":\"%s\",\"build\":\"%s\"}",
        VERSION, BUILD);
    send_json(c, buf);
}

static void handle_network_info(struct mg_connection *c) {
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
    send_json(c, j);
}

/* ---- main event handler ---- */
static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG)
        return;

    struct mg_http_message *hm = ev_data;

    if (match_uri(hm, "/health"))
        handle_health(c);
    else if (match_uri(hm, "/api/network-info"))
        handle_network_info(c);
    else if (match_uri(hm, "/v1/models"))
        send_json(c,
            "{\"object\":\"list\",\"data\":[{\"id\":\"local-model\","
            "\"object\":\"model\",\"owned_by\":\"local\"}]}");
    else if (match_uri(hm, "/props"))
        send_json(c,
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
    else if (match_uri(hm, "/slots"))
        send_json(c,
            "[{\"id\":0,\"state\":0,"
            "\"model\":\"local-model\"}]");
    else if (match_uri(hm, "/v1/health"))
        handle_health(c);
    else if (match_uri(hm, "/api/config"))
        handle_health(c);
    else if (match_uri(hm, "/v1/chat/completions"))
        handle_chat_proxy(c, hm);
    else if (match_uri(hm, "/completion"))
        handle_chat_proxy(c, hm);
    else if (match_uri(hm, "/v2"))
        mg_http_reply(c, 307, "Location:/v2/\r\n", "");
    else {
        struct mg_http_serve_opts o = {.root_dir = FRONTEND_DIR};
        mg_http_serve_dir(c, hm, &o);
    }
}

/* ---- server thread ---- */
static void *server_thread(void *arg) {
    (void)arg;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    char addr[32];
    snprintf(addr, sizeof(addr), "http://0.0.0.0:%d", BACKEND_PORT);
    mg_http_listen(&mgr, addr, ev_handler, NULL);

    g_running = 1;
    while (g_running)
        mg_mgr_poll(&mgr, 100);

    mg_mgr_free(&mgr);
    return NULL;
}

int server_start(void) {
    curl_global_init(CURL_GLOBAL_ALL);
    if (pthread_create(&g_thread, NULL, server_thread, NULL) != 0)
        return -1;
    for (int i = 0; i < 30; i++) {
        if (g_running)
            return 0;
        usleep(500000);
    }
    return -1;
}

void server_stop(void) {
    g_running = 0;
    pthread_join(g_thread, NULL);
    curl_global_cleanup();
}