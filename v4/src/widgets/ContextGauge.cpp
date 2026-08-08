#include "ContextGauge.h"
#include "../i18n.h"
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

namespace {
// rough chars-per-token for the local models (llama.cpp BPE ~4 chars/token)
constexpr double CHARS_PER_TOKEN = 4.0;
}

ContextGauge::ContextGauge(QWidget *parent) : QWidget(parent) {
    setFixedSize(20, 20);
    setToolTip(I18n::t(QStringLiteral("Context usage")));
}

void ContextGauge::setUsage(qint64 promptChars, qint64 completionChars, int maxCtxTokens) {
    m_prompt = promptChars;
    m_completion = completionChars;
    m_maxCtx = qMax(256, maxCtxTokens);
    update();
}

void ContextGauge::paintEvent(QPaintEvent *) {
    const double promptTok = m_prompt / CHARS_PER_TOKEN;
    const double compTok = m_completion / CHARS_PER_TOKEN;
    const double used = (promptTok + compTok) / m_maxCtx;
    const double pct = qBound(0.0, used, 1.0);

    // v2 colors: grey (ok) / amber-400 >= 80% / red-400 >= 95%
    QColor color = QColor(QStringLiteral("#a1a1aa"));
    if (pct >= 0.95) color = QColor(QStringLiteral("#f87171"));
    else if (pct >= 0.80) color = QColor(QStringLiteral("#fbbf24"));

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(2, 2, 16, 16);
    QPen track(QColor(128, 128, 128, 25), 3, Qt::SolidLine, Qt::RoundCap);
    p.setPen(track);
    p.drawArc(r, 210 * 16, 120 * 16); // -120°..+120° (300° arc)
    if (pct > 0.001) {
        QPen arc(color, 3, Qt::SolidLine, Qt::RoundCap);
        p.setPen(arc);
        p.drawArc(r, 210 * 16, int(120 * 16 * pct));
    }
}

void ContextGauge::mousePressEvent(QMouseEvent *event) {
    const double promptTok = m_prompt / CHARS_PER_TOKEN;
    const double compTok = m_completion / CHARS_PER_TOKEN;
    const qint64 used = qint64(promptTok + compTok);
    const qint64 remaining = qMax<qint64>(0, m_maxCtx - used);
    const int pct = int(100.0 * used / m_maxCtx);
    // v2 popup content: "Context · used/total", "% used / N remaining"
    const QString text = I18n::t(QStringLiteral("Context")) +
                         QStringLiteral(" · %1/%2\n").arg(used).arg(m_maxCtx) +
                         QStringLiteral("%1%% used · %2 %3").arg(pct).arg(remaining)
                             .arg(I18n::t(QStringLiteral("remaining")));
    QToolTip::showText(mapToGlobal(event->pos()), text, this);
}

void ContextGauge::enterEvent(QEnterEvent *) {
    update();
}

void ContextGauge::leaveEvent(QEvent *) {
    update();
}
