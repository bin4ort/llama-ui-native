/*
 * i18n.h — dict-driven translations (same source of truth as the web app:
 * frontend/v2/lang/*.json). t() with key fallback.
 */
#pragma once
#include <QString>
#include <QStringList>

namespace I18n {

/** Load all dicts from the repo lang dir (UI_SPEC_DIR/frontend/v2/lang). */
void init();

/** Available language codes (e.g. en, de, ru, ja …). */
QStringList languages();

/** Current language code. */
QString current();

/** Switch the active language. */
void setLanguage(const QString &code);

/** Translate a key (fallback: the key itself). */
QString t(const QString &key);

} // namespace I18n
