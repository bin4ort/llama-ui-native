/*
 * ContextGauge.h — spec context gauge (UI-SPEC §5.7): circular dial
 * colored by context usage (green <70%, amber <90%, red beyond), a click
 * shows a popup with prompt / history / completion rows.
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
    bool m_hover = false;
};
