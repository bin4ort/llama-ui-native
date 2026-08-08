/*
 * Config.h — persisted settings with the same keys as the web app's
 * LlamaUi.config. Stored as JSON in ~/.config/llama-ui-native/config.json
 * (the web version keeps it in WebKit localStorage; v4 uses a plain file —
 * no browser, no environment variables).
 */
#pragma once
#include <QJsonObject>
#include <QString>

namespace Config {

/** Load (or create) the config file. */
QJsonObject load();

/** Persist the whole config object. */
void save(const QJsonObject &cfg);

/** Get one setting (defaults applied; QJsonValue::Undefined if unknown). */
QJsonValue value(const QString &key);

/** Path of the config file (~/.config/llama-ui-native/config.json). */
QString path();

} // namespace Config
