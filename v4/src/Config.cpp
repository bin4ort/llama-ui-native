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
    if (f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            g_cfg = doc.object();
            return g_cfg;
        }
    }
    // Defaults (same keys/defaults as the web app's settings-store)
    g_cfg = QJsonObject{
        {QStringLiteral("language"), QStringLiteral("en")},
        {QStringLiteral("theme"), QStringLiteral("system")},
        {QStringLiteral("serverEndpoint"), QStringLiteral("http://localhost:8080")},
        {QStringLiteral("systemMessage"), QString()},
        {QStringLiteral("temperature"), QString()},
        {QStringLiteral("top_k"), QString()},
        {QStringLiteral("top_p"), QString()},
        {QStringLiteral("min_p"), QString()},
        {QStringLiteral("sendOnEnter"), true},
        {QStringLiteral("logLevel"), 2},
        {QStringLiteral("model"), QString()},
    };
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

} // namespace Config
