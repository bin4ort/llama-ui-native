/*
 * Icons.h — original v2 icons (Lucide SVG paths extracted from the
 * @lucide/svelte package; custom Logo/McpLogo marks from the v2 source).
 * Rendered natively via QSvgRenderer — no text-symbol glyphs.
 */
#pragma once
#include <QColor>
#include <QIcon>
#include <QString>

namespace Icons {

/** Initialize the icon set (loads the embedded SVG data once). */
void init();

/** Theme color used for icons; MainWindow::applyTheme keeps it in sync. */
void setColor(const QColor &color);
QColor color();

/** Lucide-style icon (24x24 viewBox, stroke = color). */
QIcon get(const QString &name, const QColor &color);

/** Custom marks (logo.svg / McpLogo from the v2 source). */
QIcon logo(const QColor &color);
QIcon mcpLogo(const QColor &color);

/** Build an icon-only push button helper: returns a pixmap. */
QPixmap pixmap(const QString &name, const QColor &color, int size);

} // namespace Icons
