/*
 * MIT License — Copyright (c) 2025 Llama UI Native
 * See LICENSE file in the project root.
 */
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include "server.h"

static void chdir_to_bindir(void) {
    char exe[PATH_MAX], real[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        exe[n] = 0;
        realpath(exe, real);
        chdir(dirname(real));
    }
}

static void on_destroy(GtkWidget *w, gpointer d) {
    (void)w;
    (void)d;
    server_stop();
    gtk_main_quit();
}

static gboolean on_webview_decide_policy(WebKitWebView *wv,
    WebKitPolicyDecision *dc, WebKitPolicyDecisionType t, gpointer d) {
    (void)d;
    if (t != WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION)
        return FALSE;

    WebKitNavigationPolicyDecision *n = (WebKitNavigationPolicyDecision *)dc;
    WebKitNavigationAction *action =
        webkit_navigation_policy_decision_get_navigation_action(n);
    const char *u = webkit_uri_request_get_uri(
        webkit_navigation_action_get_request(action));

    if (g_str_has_prefix(u, "http://localhost:8765") ||
        g_str_has_prefix(u, "http://127.0.0.1:8765")) {
        webkit_policy_decision_use(dc);
    } else {
        g_app_info_launch_default_for_uri(u, NULL, NULL);
        webkit_policy_decision_ignore(dc);
    }
    return TRUE;
}

static void on_webview_load_changed(WebKitWebView *wv,
    WebKitLoadEvent ev, gpointer d) {
    if (ev == WEBKIT_LOAD_FINISHED) {
        const char *t = webkit_web_view_get_title(wv);
        if (t && *t)
            gtk_window_set_title(GTK_WINDOW(d), t);
    }
}

int main(int argc, char *argv[]) {
    chdir_to_bindir();

    char cwd[PATH_MAX];
    char icon_path[PATH_MAX + 14];
    if (getcwd(cwd, sizeof(cwd)))
        snprintf(icon_path, sizeof(icon_path), "%s/static/icon-128.png", cwd);

    if (server_start() != 0)
        g_warning("Server failed on port %d", BACKEND_PORT);

    gtk_init(&argc, &argv);

    GtkWidget *w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(w), "Llama UI Native");
    gtk_window_set_default_size(GTK_WINDOW(w), 1440, 900);
    gtk_window_set_position(GTK_WINDOW(w), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_from_file(GTK_WINDOW(w), icon_path, NULL);

    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css,
        "window{background-color:#141414}box{background-color:#141414}",
        -1, NULL);
    GtkStyleContext *ctx = gtk_widget_get_style_context(w);
    gtk_style_context_add_provider(ctx,
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    WebKitWebView *wv = WEBKIT_WEB_VIEW(webkit_web_view_new());
    WebKitSettings *ws = webkit_web_view_get_settings(wv);
    webkit_settings_set_enable_developer_extras(ws, TRUE);
    webkit_settings_set_enable_write_console_messages_to_stdout(ws, TRUE);
    webkit_web_context_clear_cache(webkit_web_view_get_context(wv));

    GdkRGBA bg = {.08, .08, .08, 1.};
    webkit_web_view_set_background_color(wv, &bg);

    g_signal_connect(w, "destroy", G_CALLBACK(on_destroy), NULL);
    g_signal_connect(wv, "decide-policy",
        G_CALLBACK(on_webview_decide_policy), NULL);
    g_signal_connect(wv, "load-changed",
        G_CALLBACK(on_webview_load_changed), w);

    webkit_web_view_load_uri(wv, "http://localhost:8765/?native=1");
    gtk_container_add(GTK_CONTAINER(w), GTK_WIDGET(wv));
    gtk_widget_show_all(w);

    gtk_main();
    return 0;
}
