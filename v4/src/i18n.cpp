#include "i18n.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QCoreApplication>

namespace I18n {

static QMap<QString, QJsonObject> g_dicts;
static QString g_lang = QStringLiteral("en");

void init() {
    const QString root = QStringLiteral(UI_SPEC_DIR);
    QDir dir(root + QStringLiteral("/web/lang"));
    const auto files = dir.entryList({QStringLiteral("*.json")}, QDir::Files);
    for (const auto &f : files) {
        QFile file(dir.filePath(f));
        if (!file.open(QIODevice::ReadOnly)) continue;
        const auto doc = QJsonDocument::fromJson(file.readAll());
        if (doc.isObject())
            g_dicts.insert(f.chopped(5), doc.object());
    }
    if (g_dicts.contains(QStringLiteral("en"))) g_lang = QStringLiteral("en");
}

QStringList languages() {
    QStringList codes = g_dicts.keys();
    codes.sort();
    return codes;
}

QString current() { return g_lang; }

void setLanguage(const QString &code) {
    if (g_dicts.contains(code)) g_lang = code;
}

QString t(const QString &key) {
    const auto it = g_dicts.constFind(g_lang);
    if (it != g_dicts.constEnd()) {
        const auto v = it.value().value(key);
        if (v.isString()) return v.toString();
    }
    return key;
}

} // namespace I18n
