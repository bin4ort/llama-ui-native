/*
 * Presets.h — persona presets: built-in library (the approved 5) seeded on
 * first run + user presets, stored in the config (systemPromptPresets).
 */
#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace Presets {

struct Preset {
    QString id;
    QString name;
    QString description;
    QString content;
};

/** Load from config; seeds the built-in library when absent. */
QJsonArray load(const QJsonObject &cfg);

/** All presets (built-ins + user). */
QList<Preset> list(const QJsonArray &raw);

/** Add a preset, returns the new array. */
QJsonArray add(QJsonArray raw, const QString &name, const QString &description,
               const QString &content);

} // namespace Presets
