#include "Icons.h"
#include <QByteArray>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

#include "icon_data.inc"

namespace {

QHash<QString, QJsonArray> buildPathData() {
    QHash<QString, QJsonArray> d;
    for (const auto &e : Icons::kIcons) {
        const auto doc = QJsonDocument::fromJson(QByteArray(e.json));
        if (doc.isArray()) d.insert(QString::fromLatin1(e.name), doc.array());
    }
    return d;
}

const QHash<QString, QJsonArray> &pathData() {
    static const QHash<QString, QJsonArray> d = buildPathData();
    return d;
}

const char *kLogoB64 =
#include "logo_svg.b64"
    ;
const char *kMcpB64 =
#include "mcp_svg.b64"
    ;

QByteArray decode(const char *b64) {
    return QByteArray::fromBase64(QByteArray(b64));
}

/** Build the SVG markup for a lucide icon with the requested stroke color. */
QString svgFor(const QJsonArray &node, const QString &color) {
    QString body;
    for (const auto &el : node) {
        const auto arr = el.toArray();
        const QString type = arr.at(0).toString();
        const QJsonObject attrs = arr.at(1).toObject();
        QString attrsStr;
        for (auto it = attrs.begin(); it != attrs.end(); ++it)
            attrsStr += QStringLiteral(" %1=\"%2\"").arg(it.key(), it.value().toString());
        body += QStringLiteral("<%1%2/>").arg(type, attrsStr);
    }
    return QStringLiteral(
               "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"24\" height=\"24\" "
               "viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"%1\" stroke-width=\"2\" "
               "stroke-linecap=\"round\" stroke-linejoin=\"round\">%2</svg>")
        .arg(color, body);
}

QPixmap renderSvg(const QString &svg, int size) {
    QSvgRenderer renderer(svg.toUtf8());
    QPixmap pm(size, size);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();
    return pm;
}

} // namespace

namespace Icons {

static QColor g_color(229, 229, 229);

void setColor(const QColor &c) { g_color = c; }
QColor color() { return g_color; }

void init() {}

QIcon get(const QString &name, const QColor &color) {
    const auto &data = pathData();
    const auto it = data.constFind(name);
    if (it == data.constEnd()) return QIcon();
    QIcon icon;
    icon.addPixmap(renderSvg(svgFor(it.value(), color.name()), 16));
    icon.addPixmap(renderSvg(svgFor(it.value(), color.name()), 20));
    icon.addPixmap(renderSvg(svgFor(it.value(), color.name()), 24));
    icon.addPixmap(renderSvg(svgFor(it.value(), color.name()), 32));
    return icon;
}

QPixmap pixmap(const QString &name, const QColor &color, int size) {
    const auto &data = pathData();
    const auto it = data.constFind(name);
    if (it == data.constEnd()) return QPixmap();
    return renderSvg(svgFor(it.value(), color.name()), size);
}

QIcon logo(const QColor &color) {
    // the logo mark is drawn in its own colors; ignore the requested color
    Q_UNUSED(color);
    QIcon icon;
    icon.addPixmap(renderSvg(QString::fromUtf8(decode(kLogoB64)), 24));
    return icon;
}

QIcon mcpLogo(const QColor &color) {
    Q_UNUSED(color);
    QIcon icon;
    icon.addPixmap(renderSvg(QString::fromUtf8(decode(kMcpB64)), 24));
    return icon;
}

} // namespace Icons
