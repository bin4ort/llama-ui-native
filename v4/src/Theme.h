/*
 * Theme.h — spec-driven theme engine (UI-SPEC.md §1–§2).
 *
 * Converts the instruction set's oklch color tokens to sRGB and generates
 * Qt stylesheets. Supports the base light/dark themes and all 11 custom
 * palettes. No Qt environment variables are used or required — the theme is
 * applied programmatically from the config.
 */
#pragma once
#include <QColor>
#include <QString>

namespace Theme {

/** Palette id: system/light/dark + the 11 custom names. */
struct Palette {
    QString id;          // e.g. "nord", "amoled", "gruvbox-light"
    bool dark;           // palette family (dark/light)
};

/** All supported palettes in selector order (dark first, like the web UI). */
const QList<Palette> &palettes();

/** Resolve a palette id ("system" -> follows the system scheme). */
Palette resolve(const QString &id);

/** Convert a spec oklch token ("oklch(0.205 0 0)" / "oklch(1 0 0 / 30%)")
 *  to an sRGB QColor. Falls back to black on parse failure. */
QColor oklch(const QString &token);

/** Spec token lookups for a palette (token tables from UI-SPEC.md §2). */
struct Tokens {
    QColor background, foreground, card, popover;
    QColor primary, primaryForeground, secondary, secondaryForeground;
    QColor muted, mutedForeground, accent, accentForeground;
    QColor destructive, border, input, ring;
    QColor sidebar, sidebarForeground, sidebarAccent, sidebarBorder;
    QColor codeBackground, codeForeground;
};

/** Build the token set for a palette id (exact v0.4.3 values). */
Tokens tokens(const QString &paletteId);

/** Generate the full application stylesheet for a palette (UI-SPEC §1, §3). */
QString stylesheet(const QString &paletteId);

/** Base font family from the spec (§1.3). */
QString fontFamily();

} // namespace Theme
