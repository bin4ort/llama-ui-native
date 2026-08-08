/*
 * Composer.h — spec composer (UI-SPEC §3.18 / §05.1): rounded-4xl/3xl bar,
 * shadow-sm -> focus-within:shadow-md, attach/persona icon buttons and a
 * circular send button.
 */
#pragma once
#include <QJsonArray>
#include <QStringList>
#include <QWidget>

class QPushButton;
class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QComboBox;
class ContextGauge;

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

    /** Model selector (UI-SPEC §11.5); displayItems may carry tags,
     *  rawIds are the real model ids emitted on selection. */
    void setModels(const QStringList &displayItems, const QStringList &rawIds,
                   const QString &currentRaw);
    QString model() const;

    /** Persona trigger label (v2: sparkles + active name + chevron). */
    void setPersonaName(const QString &name);

    /** Context usage for the gauge (UI-SPEC §5.7). */
    void setContextUsage(qint64 promptChars, qint64 completionChars, int maxCtx);

signals:
    void sendRequested(const QString &text);
    void stopRequested();
    void personaRequested();
    void attachRequested();
    void attachmentsChanged();
    void attachmentPreviewRequested(int index);
    void modelSelected(const QString &id);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    void updateHeight();
    void rebuildChips();
    static QString formatSize(qint64 bytes);

    QTextEdit *m_input;
    QPushButton *m_send;
    QPushButton *m_persona;
    QComboBox *m_modelSelect = nullptr;
    ContextGauge *m_gauge = nullptr;
    QVBoxLayout *m_outer;
    QHBoxLayout *m_chipsRow;
    QJsonArray m_attachments;
    bool m_busy = false;
};
