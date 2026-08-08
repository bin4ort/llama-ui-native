/*
 * ContextGauge.h — v2 context gauge (ChatFormActions / context-gauge.ts):
 * 20px ring arc, grey / amber-400 (>=80%) / red-400 (>=95%), click popup
 * "Context · used/total" + "% used / N remaining". Hidden until tokens.
 */
#pragma once
#include <QWidget>

class ContextGauge : public QWidget {
    Q_OBJECT
public:
    explicit ContextGauge(QWidget *parent = nullptr);

    /** promptTokens + completionTokens in the current conversation. */
    void setUsage(qint64 promptChars, qint64 completionChars, int maxCtxTokens);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    qint64 m_prompt = 0;
    qint64 m_completion = 0;
    int m_maxCtx = 4096;
};
