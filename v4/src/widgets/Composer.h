/*
 * Composer.h — spec composer (UI-SPEC §3.18 / §05.1): rounded-4xl/3xl bar,
 * shadow-sm -> focus-within:shadow-md, attach/persona icon buttons and a
 * circular send button.
 */
#pragma once
#include <QWidget>

class QPushButton;
class QTextEdit;

class Composer : public QWidget {
    Q_OBJECT
public:
    explicit Composer(QWidget *parent = nullptr);

    QString text() const;
    void clear();
    void setBusy(bool busy);
    void focusInput();
    void setTextForEdit(const QString &text);

signals:
    void sendRequested(const QString &text);
    void stopRequested();
    void personaRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void updateHeight();

    QTextEdit *m_input;
    QPushButton *m_send;
    QPushButton *m_persona;
    bool m_busy = false;
};
