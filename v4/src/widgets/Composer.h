/*
 * Composer.h — spec composer (UI-SPEC §3.18 / §05.1): rounded-4xl/3xl bar,
 * shadow-sm -> focus-within:shadow-md, attach/persona icon buttons and a
 * circular send button.
 */
#pragma once
#include <QJsonArray>
#include <QWidget>

class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;

class Composer : public QWidget {
    Q_OBJECT
public:
    explicit Composer(QWidget *parent = nullptr);

    QString text() const;
    void clear();
    void setBusy(bool busy);
    void focusInput();
    void setTextForEdit(const QString &text);

    /** Pending attachments as [{name, mime, data(base64), image}] */
    const QJsonArray &attachments() const { return m_attachments; }
    void addAttachment(const QJsonObject &att);
    void setAttachments(const QJsonArray &atts);

signals:
    void sendRequested(const QString &text);
    void stopRequested();
    void personaRequested();
    void attachRequested();
    void attachmentsChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void updateHeight();
    void rebuildChips();

    QTextEdit *m_input;
    QPushButton *m_send;
    QPushButton *m_persona;
    QVBoxLayout *m_outer;
    QHBoxLayout *m_chipsRow;
    QJsonArray m_attachments;
    bool m_busy = false;
};
