/*
 * Theme.cpp — spec token tables (UI-SPEC.md §2, verbatim oklch values) +
 * oklch->sRGB conversion + QSS generation (UI-SPEC.md §1, §3).
 */
#include "Theme.h"
#include <QRegularExpression>
#include <QtMath>

namespace Theme {

static QList<Palette> g_palettes = {
    {"nord", true},           {"dracula", true},   {"cobalt", true},
    {"solarized", true},      {"gruvbox", true},   {"amoled", true},
    {"ionized-purple", true}, {"ionized-red", true}, {"ionized-cyan", true},
    {"snow", false},          {"gruvbox-light", false},
};

const QList<Palette> &palettes() { return g_palettes; }

Palette resolve(const QString &id) {
    for (const auto &p : g_palettes)
        if (p.id == id) return p;
    return {id == QStringLiteral("dark") ? QStringLiteral("dark") : QStringLiteral("system"),
            id != QStringLiteral("light")};
}

/* ---- oklch -> sRGB (per OKLab spec) ---- */
static double srgbCompand(double c) {
    c = qBound(0.0, c, 1.0);
    return c <= 0.0031308 ? 12.92 * c : 1.055 * qPow(c, 1.0 / 2.4) - 0.055;
}

QColor oklch(const QString &token) {
    QRegularExpression re(
        R"(oklch\(\s*([0-9.]+)\s+([0-9.]+)\s+([0-9.]+)\s*(?:\/\s*([0-9.]+)%?)?\s*\))");
    auto match = re.match(token);
    if (!match.hasMatch()) return QColor(Qt::black);
    const double L = match.captured(1).toDouble();
    const double C = match.captured(2).toDouble();
    const double H = qDegreesToRadians(match.captured(3).toDouble());
    const double alpha = match.captured(4).isEmpty() ? 1.0 : match.captured(4).toDouble() / 100.0;

    const double a = C * qCos(H);
    const double b = C * qSin(H);

    const double l_ = L + 0.3963377774 * a + 0.2158037573 * b;
    const double m_ = L - 0.1055613458 * a - 0.0638541728 * b;
    const double s_ = L - 0.0894841775 * a - 1.2914855480 * b;
    const double l = l_ * l_ * l_;
    const double m = m_ * m_ * m_;
    const double s = s_ * s_ * s_;

    double r = 4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s;
    double g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s;
    double bl = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s;

    return QColor::fromRgbF(srgbCompand(r), srgbCompand(g), srgbCompand(bl), alpha);
}

/* ---- token tables (verbatim from UI-SPEC.md §2) ---- */
static QColor t(const Tokens &tk, QColor Tokens::*m) { return tk.*m; }

Tokens tokens(const QString &paletteId) {
    Tokens tk;
    const bool light = paletteId == QStringLiteral("light") || paletteId == QStringLiteral("snow") ||
                       paletteId == QStringLiteral("gruvbox-light");
    const bool system = paletteId == QStringLiteral("system");

    // Light (:root)
    if (light || system) {
        tk.background = oklch(QStringLiteral("oklch(1 0 0)"));
        tk.foreground = oklch(QStringLiteral("oklch(0.145 0 0)"));
        tk.card = oklch(QStringLiteral("oklch(1 0 0)"));
        tk.popover = oklch(QStringLiteral("oklch(1 0 0)"));
        tk.primary = oklch(QStringLiteral("oklch(0.205 0 0)"));
        tk.primaryForeground = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.secondary = oklch(QStringLiteral("oklch(0.95 0 0)"));
        tk.secondaryForeground = oklch(QStringLiteral("oklch(0.205 0 0)"));
        tk.muted = oklch(QStringLiteral("oklch(0.97 0 0)"));
        tk.mutedForeground = oklch(QStringLiteral("oklch(0.556 0 0)"));
        tk.accent = oklch(QStringLiteral("oklch(0.95 0 0)"));
        tk.accentForeground = oklch(QStringLiteral("oklch(0.205 0 0)"));
        tk.destructive = oklch(QStringLiteral("oklch(0.577 0.245 27.325)"));
        tk.border = oklch(QStringLiteral("oklch(0.875 0 0)"));
        tk.input = oklch(QStringLiteral("oklch(0.92 0 0)"));
        tk.ring = oklch(QStringLiteral("oklch(0.708 0 0)"));
        tk.sidebar = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.sidebarForeground = oklch(QStringLiteral("oklch(0.145 0 0)"));
        tk.sidebarAccent = oklch(QStringLiteral("oklch(0.97 0 0)"));
        tk.sidebarBorder = oklch(QStringLiteral("oklch(0.922 0 0)"));
        tk.codeBackground = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.codeForeground = oklch(QStringLiteral("oklch(0.145 0 0)"));
    } else {
        // Dark (default base for dark-family palettes)
        tk.background = oklch(QStringLiteral("oklch(0.16 0 0)"));
        tk.foreground = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.card = oklch(QStringLiteral("oklch(0.205 0 0)"));
        tk.popover = oklch(QStringLiteral("oklch(0.205 0 0)"));
        tk.primary = oklch(QStringLiteral("oklch(0.922 0 0)"));
        tk.primaryForeground = oklch(QStringLiteral("oklch(0.205 0 0)"));
        tk.secondary = oklch(QStringLiteral("oklch(0.29 0 0)"));
        tk.secondaryForeground = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.muted = oklch(QStringLiteral("oklch(0.269 0 0)"));
        tk.mutedForeground = oklch(QStringLiteral("oklch(0.708 0 0)"));
        tk.accent = oklch(QStringLiteral("oklch(0.269 0 0)"));
        tk.accentForeground = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.destructive = oklch(QStringLiteral("oklch(0.704 0.191 22.216)"));
        tk.border = oklch(QStringLiteral("oklch(1 0 0 / 30%)"));
        tk.input = oklch(QStringLiteral("oklch(1 0 0 / 30%)"));
        tk.ring = oklch(QStringLiteral("oklch(0.556 0 0)"));
        tk.sidebar = oklch(QStringLiteral("oklch(0.2 0 0)"));
        tk.sidebarForeground = oklch(QStringLiteral("oklch(0.985 0 0)"));
        tk.sidebarAccent = oklch(QStringLiteral("oklch(0.269 0 0)"));
        tk.sidebarBorder = oklch(QStringLiteral("oklch(1 0 0 / 10%)"));
        tk.codeBackground = oklch(QStringLiteral("oklch(0.225 0 0)"));
        tk.codeForeground = oklch(QStringLiteral("oklch(0.875 0 0)"));
    }

    // Palette deltas (UI-SPEC.md §2.3)
    auto apply = [&](QColor Tokens::*m, const QString &tok) { tk.*m = oklch(tok); };
    if (paletteId == QStringLiteral("nord")) {
        apply(&Tokens::background, "oklch(0.25 0.01 252)"); apply(&Tokens::foreground, "oklch(0.88 0.02 240)");
        apply(&Tokens::card, "oklch(0.27 0.01 252)"); apply(&Tokens::popover, "oklch(0.27 0.01 252)");
        apply(&Tokens::primary, "oklch(0.80 0.04 225)"); apply(&Tokens::primaryForeground, "oklch(0.985 0 0)");
        apply(&Tokens::accent, "oklch(0.65 0.07 195)"); apply(&Tokens::accentForeground, "oklch(0.985 0 0)");
        apply(&Tokens::mutedForeground, "oklch(0.6 0.03 240)"); apply(&Tokens::border, "oklch(0.88 0.02 240 / 15%)");
        apply(&Tokens::input, "oklch(0.88 0.02 240 / 15%)"); apply(&Tokens::ring, "oklch(0.65 0.07 195)");
        apply(&Tokens::sidebar, "oklch(0.22 0.01 252)"); apply(&Tokens::codeBackground, "oklch(0.22 0.01 252)");
    } else if (paletteId == QStringLiteral("dracula")) {
        apply(&Tokens::background, "oklch(0.22 0.02 290)"); apply(&Tokens::foreground, "oklch(0.96 0.01 290)");
        apply(&Tokens::card, "oklch(0.24 0.02 290)"); apply(&Tokens::primary, "oklch(0.8 0.1 315)");
        apply(&Tokens::accent, "oklch(0.7 0.15 315)"); apply(&Tokens::mutedForeground, "oklch(0.55 0.03 280)");
        apply(&Tokens::border, "oklch(0.96 0.01 290 / 15%)"); apply(&Tokens::input, "oklch(0.96 0.01 290 / 15%)");
        apply(&Tokens::ring, "oklch(0.7 0.15 315)"); apply(&Tokens::codeBackground, "oklch(0.2 0.02 290)");
    } else if (paletteId == QStringLiteral("cobalt")) {
        apply(&Tokens::background, "oklch(0.22 0.025 255)"); apply(&Tokens::foreground, "oklch(0.92 0.01 255)");
        apply(&Tokens::card, "oklch(0.24 0.025 255)"); apply(&Tokens::primary, "oklch(0.78 0.1 75)");
        apply(&Tokens::accent, "oklch(0.72 0.14 75)"); apply(&Tokens::border, "oklch(0.92 0.01 255 / 10%)");
        apply(&Tokens::input, "oklch(0.92 0.01 255 / 10%)"); apply(&Tokens::ring, "oklch(0.72 0.14 75)");
        apply(&Tokens::codeBackground, "oklch(0.2 0.025 255)");
    } else if (paletteId == QStringLiteral("solarized")) {
        apply(&Tokens::background, "oklch(0.24 0.02 205)"); apply(&Tokens::foreground, "oklch(0.78 0.03 190)");
        apply(&Tokens::card, "oklch(0.26 0.02 205)"); apply(&Tokens::primary, "oklch(0.55 0.1 185)");
        apply(&Tokens::primaryForeground, "oklch(0.97 0.01 100)"); apply(&Tokens::accent, "oklch(0.68 0.15 55)");
        apply(&Tokens::mutedForeground, "oklch(0.62 0.04 80)"); apply(&Tokens::border, "oklch(0.78 0.03 190 / 12%)");
        apply(&Tokens::input, "oklch(0.78 0.03 190 / 12%)"); apply(&Tokens::ring, "oklch(0.68 0.15 55)");
        apply(&Tokens::codeBackground, "oklch(0.22 0.02 205)");
    } else if (paletteId == QStringLiteral("gruvbox")) {
        apply(&Tokens::background, "oklch(0.23 0.02 85)"); apply(&Tokens::foreground, "oklch(0.88 0.04 85)");
        apply(&Tokens::card, "oklch(0.25 0.02 85)"); apply(&Tokens::primary, "oklch(0.78 0.12 80)");
        apply(&Tokens::accent, "oklch(0.75 0.14 85)"); apply(&Tokens::muted, "oklch(0.4 0.02 80)");
        apply(&Tokens::border, "oklch(0.88 0.04 85 / 15%)"); apply(&Tokens::input, "oklch(0.88 0.04 85 / 15%)");
        apply(&Tokens::ring, "oklch(0.75 0.14 85)"); apply(&Tokens::codeBackground, "oklch(0.21 0.02 85)");
    } else if (paletteId == QStringLiteral("amoled")) {
        apply(&Tokens::background, "oklch(0.08 0 0)"); apply(&Tokens::foreground, "oklch(0.96 0 0)");
        apply(&Tokens::card, "oklch(0.12 0 0)"); apply(&Tokens::primary, "oklch(0.88 0 0)");
        apply(&Tokens::primaryForeground, "oklch(0.12 0 0)"); apply(&Tokens::accent, "oklch(0.18 0 0)");
        apply(&Tokens::mutedForeground, "oklch(0.62 0 0)"); apply(&Tokens::border, "oklch(0.96 0 0 / 15%)");
        apply(&Tokens::input, "oklch(0.96 0 0 / 15%)"); apply(&Tokens::ring, "oklch(0.45 0 0)");
        apply(&Tokens::codeBackground, "oklch(0.1 0 0)");
    } else if (paletteId == QStringLiteral("snow")) {
        apply(&Tokens::background, "oklch(0.97 0.02 70)"); apply(&Tokens::foreground, "oklch(0.2 0.03 70)");
        apply(&Tokens::card, "oklch(0.99 0.01 70)"); apply(&Tokens::primary, "oklch(0.38 0.06 65)");
        apply(&Tokens::primaryForeground, "oklch(0.98 0.02 70)"); apply(&Tokens::secondary, "oklch(0.93 0.03 68)");
        apply(&Tokens::accent, "oklch(0.9 0.04 60)"); apply(&Tokens::accentForeground, "oklch(0.25 0.05 70)");
        apply(&Tokens::muted, "oklch(0.94 0.03 68)"); apply(&Tokens::mutedForeground, "oklch(0.48 0.05 65)");
        apply(&Tokens::border, "oklch(0.2 0.03 70 / 12%)"); apply(&Tokens::input, "oklch(0.2 0.03 70 / 12%)");
        apply(&Tokens::ring, "oklch(0.38 0.06 65)"); apply(&Tokens::codeBackground, "oklch(0.96 0.02 70)");
    } else if (paletteId == QStringLiteral("gruvbox-light")) {
        apply(&Tokens::background, "oklch(0.95 0.03 85)"); apply(&Tokens::foreground, "oklch(0.22 0.04 85)");
        apply(&Tokens::card, "oklch(0.97 0.02 85)"); apply(&Tokens::primary, "oklch(0.45 0.08 70)");
        apply(&Tokens::accent, "oklch(0.85 0.06 70)"); apply(&Tokens::muted, "oklch(0.88 0.03 85)");
        apply(&Tokens::border, "oklch(0.22 0.04 85 / 12%)"); apply(&Tokens::input, "oklch(0.22 0.04 85 / 12%)");
        apply(&Tokens::ring, "oklch(0.45 0.08 70)"); apply(&Tokens::codeBackground, "oklch(0.93 0.03 85)");
    } else if (paletteId == QStringLiteral("ionized-purple")) {
        apply(&Tokens::primary, "oklch(0.55 0.12 315)"); apply(&Tokens::accent, "oklch(0.5 0.15 315)");
        apply(&Tokens::mutedForeground, "oklch(0.65 0.06 315)"); apply(&Tokens::border, "oklch(0.45 0.12 315 / 16%)");
        apply(&Tokens::input, "oklch(0.45 0.12 315 / 16%)"); apply(&Tokens::ring, "oklch(0.5 0.15 315)");
        apply(&Tokens::codeForeground, "oklch(0.78 0.04 315)");
    } else if (paletteId == QStringLiteral("ionized-red")) {
        apply(&Tokens::primary, "oklch(0.52 0.12 22)"); apply(&Tokens::accent, "oklch(0.48 0.16 20)");
        apply(&Tokens::mutedForeground, "oklch(0.62 0.06 20)"); apply(&Tokens::border, "oklch(0.42 0.12 20 / 16%)");
        apply(&Tokens::input, "oklch(0.42 0.12 20 / 16%)"); apply(&Tokens::ring, "oklch(0.48 0.16 20)");
        apply(&Tokens::codeForeground, "oklch(0.78 0.04 20)");
    } else if (paletteId == QStringLiteral("ionized-cyan")) {
        apply(&Tokens::primary, "oklch(0.58 0.12 245)"); apply(&Tokens::accent, "oklch(0.52 0.15 245)");
        apply(&Tokens::mutedForeground, "oklch(0.65 0.06 245)"); apply(&Tokens::border, "oklch(0.45 0.12 245 / 16%)");
        apply(&Tokens::input, "oklch(0.45 0.12 245 / 16%)"); apply(&Tokens::ring, "oklch(0.52 0.15 245)");
        apply(&Tokens::codeForeground, "oklch(0.78 0.06 245)");
    }
    return tk;
}

QString fontFamily() {
    return QStringLiteral(
        "\"Noto Sans\", ui-sans-serif, system-ui, sans-serif, "
        "\"Apple Color Emoji\", \"Segoe UI Emoji\", \"Segoe UI Symbol\"");
}

/* ---- QSS generation (UI-SPEC §1 radii, §3 component styles) ---- */
static QString hex(const QColor &c) { return c.name(QColor::HexArgb); }

QString stylesheet(const QString &paletteId) {
    const Tokens t = tokens(paletteId);
    // user bubble = primary at 5% over background (UI-SPEC §3.4)
    const auto alphaMix = [](const QColor &fg, const QColor &bg, double a) {
        return QColor::fromRgbF(fg.redF() * a + bg.redF() * (1.0 - a),
                                fg.greenF() * a + bg.greenF() * (1.0 - a),
                                fg.blueF() * a + bg.blueF() * (1.0 - a),
                                1.0);
    };
    const QColor userBubble = alphaMix(t.primary, t.background, 0.05);
    // v2 form: bg-muted/60 (light) / bg-muted/75 (dark), border/20
    const bool dark = t.background.lightnessF() < 0.5;
    const QColor formBg = alphaMix(t.muted, t.background, dark ? 0.75 : 0.60);
    const QColor formBorder = alphaMix(t.border, t.background, 0.20);
    const QString s = QStringLiteral(R"QSS(
* { font-family: %1; font-size: 14px; }
QMainWindow, QDialog { background-color: %2; color: %3; }

/* Sidebar (UI-SPEC §3.17) — collapsed icon strip */
#sidebar { background-color: %4; border: 1px solid %5; border-radius: 16px; }
#sidebar QPushButton[strip="true"] {
  background: transparent; border: none; border-radius: 8px;
  color: %6; font-size: 14px;
}
#sidebar QPushButton[strip="true"]:hover { background-color: %7; color: %3; }
#sidebar QPushButton[strip="true"][active="true"] { background-color: %8; color: %9; }
#sidebar QLabel { color: %6; }

/* Chat area */
#chatRoot { background-color: %2; }
#messages { background-color: %2; }
#greetingTitle { font-size: 30px; font-weight: 600; letter-spacing: -0.02em; color: %3; }
#greetingSub { font-size: 18px; color: %10; }

/* Composer (v2 ChatForm: bg-muted/60 dark:bg-muted/75, border/20, radius 24) */
#composer {
  background-color: %25; border: 1px solid %26; border-radius: 24px;
}
#composer:focus-within { border-color: %12; }
#composer QTextEdit {
  background: transparent; border: none; color: %3; font-size: 14px;
}
#composer QPushButton[round="true"] {
  background: transparent; border: none; border-radius: 16px;
  color: %6; min-width: 32px; max-width: 32px; min-height: 32px; max-height: 32px;
}
#composer QPushButton[round="true"]:hover { background-color: %7; color: %3; }
#composer QPushButton[send="true"] {
  background-color: %13; color: %14; border: none; border-radius: 24px;
  min-width: 48px; max-width: 48px; min-height: 48px; max-height: 48px;
  font-size: 18px;
}
#composer QPushButton[send="true"]:hover { background-color: %15; }
#composer QPushButton[send="true"]:disabled { background-color: %16; }

/* Message bubbles (UI-SPEC §3.3–3.5) */
#userBubble {
  background-color: %17; border: none; border-radius: 18px;
  color: %3; padding: 8px 12px; max-width: 1000px;
}
#assistantText { color: %3; font-size: 14px; line-height: 1.5; }
#systemBubble {
  background-color: %18; border: 2px dashed %5; border-radius: 18px;
  color: %3; padding: 8px 12px; max-width: 1000px;
}

/* Inputs (UI-SPEC §3.2) */
QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
  background-color: %19; border: 1px solid %20; border-radius: 8px;
  color: %3; padding: 6px 10px; min-height: 24px; font-size: 14px;
  selection-background-color: %13; selection-color: %14;
}
QLineEdit:focus, QComboBox:focus { border-color: %12; }
QLineEdit::placeholder { color: %10; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView {
  background-color: %21; color: %3; border: 1px solid %5; border-radius: 8px;
  selection-background-color: %8; selection-color: %9;
}

/* Checkbox (UI-SPEC §3.5) */
QCheckBox::indicator {
  width: 16px; height: 16px; border: 1px solid %20; border-radius: 4px;
  background-color: %22; 
}
QCheckBox::indicator:checked {
  background-color: %13; border-color: %13;
}
QCheckBox { color: %3; font-size: 14px; spacing: 8px; }

/* Buttons (UI-SPEC §3.1) */
QPushButton {
  background-color: %13; color: %14; border: none; border-radius: 8px;
  padding: 8px 16px; font-size: 14px; font-weight: 500;
}
QPushButton:hover { background-color: %15; }
QPushButton:disabled { background-color: %16; color: %10; }
QPushButton[outline="true"] {
  background-color: transparent; border: 1px solid %5; color: %3;
}
QPushButton[outline="true"]:hover { background-color: %7; }
QPushButton[ghost="true"] {
  background-color: transparent; color: %10; border: none;
}
QPushButton[ghost="true"]:hover { background-color: %7; color: %3; }
QPushButton[destructive="true"] { background-color: %23; color: white; }
QPushButton[destructive="true"]:hover { background-color: %24; }
QPushButton[primary="true"] { background-color: %13; color: %14; }
QPushButton[primary="true"]:hover { background-color: %15; }

/* Tool call rows (v2 ToolCallBlock header) */
#toolCallRow {
  color: %13; font-size: 12px; font-weight: 600; padding: 4px 0;
}
#messageActions { background-color: transparent; }
#msgTimestamp { color: %10; font-size: 12px; padding: 0 2px; }
#chatRow { background-color: transparent; }

/* Tool result rows (v2 ToolCallBlock output) */
#toolResultRow {
  background-color: rgba(128,128,128,0.10); border: 1px solid %5;
  border-radius: 8px; color: %10; padding: 6px 10px;
  font-size: 12px; font-family: monospace; max-width: 90%;
}

/* MCP server cards (UI-SPEC §10.2) */
#mcpCard { background-color: %11; border: 1px solid %5; border-radius: 10px; }
#mcpName { font-weight: 600; font-size: 14px; }
#mcpUrl { color: %10; font-size: 12px; }
#mcpState { font-size: 11px; padding: 1px 8px; border-radius: 8px; color: %10; }
#mcpState[on="true"] { color: #2ecc71; }
#mcpState[on="false"] { color: %23; }
#mcpEmptyTitle { font-size: 20px; font-weight: 600; color: %3; }
#mcpEmptyDesc { font-size: 14px; color: %10; }
#mcpEmptyIcon {
  background-color: %7; border-radius: 24px; color: %3; font-size: 20px;
}
#mcpHeader { border-bottom: 1px solid %5; }
#mcpTitle { font-size: 22px; font-weight: 600; letter-spacing: -0.02em; }

QPushButton[persona="true"] {
  background-color: transparent; border: 1px solid %5; border-radius: 14px;
  color: %10; padding: 4px 10px; font-size: 12px;
}
QPushButton[persona="true"]:hover { background-color: %7; color: %3; }
QPushButton[persona="true"][active="true"] { color: #fbbf24; }

/* Attachment chips (UI-SPEC §4.2) */
#attChip {
  background-color: %7; border: 1px solid %5; border-radius: 10px;
}
#attChipLabel { color: %3; font-size: 12px; }
#attChip QPushButton { min-width: 18px; max-width: 18px; min-height: 18px; max-height: 18px; padding: 0; }

/* Greeting suggestion chips (UI-SPEC §6.2) */
QPushButton[chip="true"] {
  background-color: transparent; border: 1px solid %5; border-radius: 16px;
  color: %3; padding: 8px 14px; font-size: 13px;
}
QPushButton[chip="true"]:hover { background-color: %7; border-color: %12; }

/* Server error banner (UI-SPEC §6.6) */
#bannerHost { background-color: %23; border-radius: 10px; padding: 2px; }
#errorBanner { color: white; font-size: 13px; padding: 4px 8px; }
#bannerHost QPushButton[outline="true"] { border-color: rgba(255,255,255,0.5); color: white; }

/* Reasoning block (UI-SPEC §3.6) */
QPushButton[reasoningToggle="true"] {
  background: transparent; border: none; color: %10; font-size: 13px;
  padding: 2px 4px; text-align: left; font-weight: 600;
}
QPushButton[reasoningToggle="true"]:hover { color: %3; }
#reasoningText {
  color: %10; font-size: 13px; border-left: 2px solid %5;
  padding-left: 10px;
}

/* System message pill (UI-SPEC §3.3) */
#systemPill {
  color: %10; font-size: 11px; font-weight: 600;
  background-color: %7; border-radius: 8px; padding: 2px 8px;
  max-width: 160px;
}

/* Sidebar footer + headers (UI-SPEC §2.5) */
#sidebarFooter { border-top: 1px solid %5; }
#statusDot { color: %23; font-size: 10px; }
#statusDot[state="online"] { color: #2ecc71; }
#statusLabel { color: %10; font-size: 11px; }
#versionLabel { color: %10; font-size: 11px; opacity: 0.5; }
#convHeader { color: %10; font-size: 11px; font-weight: 600; padding: 6px 4px 2px 4px; }
#convEmpty { color: %10; font-size: 13px; padding: 12px 4px; }
QPushButton[convRow="true"][active="true"] {
  background-color: %8; color: %9; font-weight: 600;
}

/* Expanded sidebar (UI-SPEC §3.17) */
#sidebarExpanded { background-color: %4; border: 1px solid %5; border-radius: 16px; }
QPushButton[convRow="true"] {
  background: transparent; border: none; border-radius: 8px;
  color: %3; text-align: left; padding: 6px 10px;
}
QPushButton[convRow="true"]:hover { background-color: %7; }
QPushButton[full="true"] { width: 100%%; }

/* Models selector trigger (v2 ModelsSelectorDropdown) */
#modelSelect {
  background-color: %19; border: 1px solid %20; border-radius: 8px;
  color: %3;
}
#modelSelect:hover { border-color: %12; }
#modelList { background: transparent; border: none; font-size: 13px; }
#modelList::item { padding: 6px 8px; border-radius: 6px; }
#modelList::item:selected { background-color: %7; color: %3; }

/* Settings page (v2 SettingsChatDesktopSidebar + section cards) */
#settingsNav { background-color: %21; border-right: 1px solid %5; }
#settingsHeading { font-size: 18px; font-weight: 600; padding: 4px 8px; }
#settingsNavList { background: transparent; font-size: 14px; }
#settingsNavList::item { padding: 8px 10px; border-radius: 8px; margin: 1px 0; }
#settingsNavList::item:selected { background-color: %7; color: %3; }
#settingsVersion { color: %10; font-size: 11px; padding: 4px 8px; }
#settingsSectionTitle { font-size: 24px; font-weight: 600; letter-spacing: -0.02em; padding: 4px 0; }
#settingsCard {
  background-color: %11; border: 1px solid %5; border-radius: 10px;
}
#settingsFieldLabel { color: %3; font-size: 14px; font-weight: 500; }
#settingsHelp { color: %10; font-size: 12px; }
#settingsFooter { border-top: 1px solid %5; background-color: %21; padding: 8px 24px; }
#settingsFooter QPushButton { padding: 4px 14px; min-height: 28px; font-size: 13px; }

/* Dialogs (UI-SPEC §3.7) */
QDialog#appDialog { background-color: %21; border: 1px solid %5; border-radius: 10px; }
QDialog QLabel#dialogTitle { font-size: 18px; font-weight: 600; }
QDialog QLabel#dialogDesc { color: %10; font-size: 14px; }

/* Scrollbars */
QScrollBar:vertical { background: transparent; width: 8px; margin: 0; }
QScrollBar::handle:vertical { background-color: %5; border-radius: 4px; min-height: 24px; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
)QSS");

    return s.arg(fontFamily(),              // 1
                 hex(t.background),         // 2
                 hex(t.foreground),         // 3
                 hex(t.sidebar),            // 4
                 hex(t.sidebarBorder),      // 5
                 hex(t.mutedForeground),    // 6
                 hex(t.accent),             // 7
                 hex(t.sidebarAccent),      // 8
                 hex(t.sidebarForeground),  // 9
                 hex(t.mutedForeground),    // 10
                 hex(t.card),               // 11
                 hex(t.ring),               // 12
                 hex(t.primary),            // 13
                 hex(t.primaryForeground),  // 14
                 hex(t.primary.lighter(112)), // 15 (hover)
                 hex(t.muted),              // 16
                 hex(userBubble),           // 17 (primary/5)
                 hex(t.muted),              // 18
                 hex(t.input),              // 19
                 hex(t.input),              // 20
                 hex(t.popover),            // 21
                 hex(t.card),               // 22
                 hex(t.destructive),        // 23
                 hex(t.destructive.lighter(115)), // 24
                 hex(formBg),               // 25 (composer bg)
                 hex(formBorder));          // 26 (composer border)
}

} // namespace Theme
