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
    setFixedSize(34, 34);
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

    QColor color;
    if (pct < 0.7) color = QColor(QStringLiteral("#2ecc71"));       // green
    else if (pct < 0.9) color = QColor(QStringLiteral("#f1c40f"));  // amber
    else color = QColor(QStringLiteral("#e74c3c"));                 // red

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r = QRectF(3, 3, 28, 28);
    QPen track(QColor(128, 128, 128, 60), 3, Qt::SolidLine, Qt::RoundCap);
    p.setPen(track);
    p.drawArc(r, 210 * 16, 120 * 16); // -120°..+120° (300° arc)
    if (pct > 0.001) {
        QPen arc(color, 3, Qt::SolidLine, Qt::RoundCap);
        p.setPen(arc);
        p.drawArc(r, 210 * 16, int(120 * 16 * pct));
    }
    if (m_hover) {
        p.setPen(QPen(QColor(128, 128, 128, 80), 1));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(r);
    }
    const QFont f = font();
    p.setFont([&] { QFont f2 = f; f2.setPixelSize(9); return f2; }());
    p.setPen(color);
    p.drawText(r, Qt::AlignCenter, QString::number(int(pct * 100)));
}

void ContextGauge::mousePressEvent(QMouseEvent *event) {
    const double promptTok = m_prompt / CHARS_PER_TOKEN;
    const double compTok = m_completion / CHARS_PER_TOKEN;
    const QString text = I18n::t(QStringLiteral("Prompt")) +
                         QStringLiteral("  %1 tk\n").arg(qint64(promptTok)) +
                         I18n::t(QStringLiteral("History")) +
                         QStringLiteral("  %1 tk\n").arg(qint64(m_completion)) +
                         I18n::t(QStringLiteral("Completion")) +
                         QStringLiteral("  %1 tk\n").arg(qint64(m_completion)) +
                         I18n::t(QStringLiteral("Context")) +
                         QStringLiteral("  %1/%2 tk").arg(qint64(promptTok + compTok)).arg(m_maxCtx);
    QToolTip::showText(mapToGlobal(event->pos()), text, this);
}

void ContextGauge::enterEvent(QEnterEvent *) {
    m_hover = true;
    update();
}

void ContextGauge::leaveEvent(QEvent *) {
    m_hover = false;
    update();
}
