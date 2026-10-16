#include "Config.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

namespace Config {

static QJsonObject g_cfg;

QString path() {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(base + QStringLiteral("/llama-ui-native"));
    return base + QStringLiteral("/llama-ui-native/config.json");
}

QJsonObject load() {
    QFile f(path());
    // Defaults (same keys/defaults as the web app's settings-store)
    QJsonObject defaults{
        {QStringLiteral("language"), QStringLiteral("en")},
        {QStringLiteral("theme"), QStringLiteral("dark")},
        {QStringLiteral("serverEndpoint"), QStringLiteral("http://127.0.0.1:8080")},
        {QStringLiteral("systemMessage"), QString()},
        {QStringLiteral("temperature"), QString()},
        {QStringLiteral("top_k"), QString()},
        {QStringLiteral("top_p"), QString()},
        {QStringLiteral("min_p"), QString()},
        {QStringLiteral("sendOnEnter"), true},
        {QStringLiteral("pasteLongTextToFileLen"), 2500.0},
        {QStringLiteral("autoMicOnEmpty"), false},
        {QStringLiteral("enableContinueGeneration"), false},
        {QStringLiteral("titleGenerationUseFirstLine"), true},
        {QStringLiteral("titleGenerationPrompt"),
         QStringLiteral("Based on the following interaction, generate a short, concise "
                        "title (maximum 6-8 words) that captures the main topic. Return "
                        "ONLY the title text, nothing else. Do not use quotes.\n\n"
                        "User: {{USER}}\n\nAssistant: {{ASSISTANT}}\n\nTitle:")},
        {QStringLiteral("copyTextAttachmentsAsPlainText"), false},
        {QStringLiteral("pdfAsImage"), false},
        {QStringLiteral("maxImageMPixels"), 0.0},
        {QStringLiteral("showSystemMessage"), true},
        {QStringLiteral("logLevel"), 2},
        {QStringLiteral("model"), QString()},
        // v2 capture defaults (settings-store)
        {QStringLiteral("showMessageStats"), false},
        {QStringLiteral("showThoughtInProgress"), true},
        {QStringLiteral("alwaysShowToolCallContent"), false},
        {QStringLiteral("renderUserContentAsMarkdown"), false},
        {QStringLiteral("renderThinkingAsMarkdown"), true},
        {QStringLiteral("fullHeightCodeBlocks"), false},
        {QStringLiteral("disableAutoScroll"), false},
        {QStringLiteral("alwaysShowSidebarOnDesktop"), false},
        {QStringLiteral("showRawModelNames"), false},
        {QStringLiteral("showModelQuantization"), true},
        {QStringLiteral("showModelTags"), true},
    };
    if (f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            // merge: file wins, defaults fill missing keys
            const QJsonObject saved = doc.object();
            for (auto it = saved.begin(); it != saved.end(); ++it)
                defaults.insert(it.key(), it.value());
            g_cfg = defaults;
            return g_cfg;
        }
    }
    g_cfg = defaults;
    save(g_cfg);
    return g_cfg;
}

void save(const QJsonObject &cfg) {
    g_cfg = cfg;
    QFile f(path());
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(cfg).toJson(QJsonDocument::Indented));
    }
}

QJsonValue value(const QString &key) {
    if (g_cfg.isEmpty()) load();
    return g_cfg.value(key);
}

} // namespace Config
