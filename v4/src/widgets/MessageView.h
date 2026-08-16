/*
 * MessageView.h — spec message rendering (UI-SPEC §3.3–3.5, §07): user
 * bubble (primary/5), assistant full-width text with hover actions
 * (copy/regenerate/edit/delete), collapsible reasoning block, greeting
 * with suggestion chips.
 */
#pragma once
#include <QJsonArray>
#include <QScrollArea>
#include <QString>
#include <QVector>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
class QLabel;
class QVBoxLayout;
class QPushButton;

struct ChatMessage {
    enum Role { User, Assistant, System } role = Assistant;
    QString id;
    QString content;
    QString type;      // text / persona / system
    QJsonArray attachments;
    bool streaming = false;
};

class MessageView : public QScrollArea {
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = nullptr);

    void addMessage(const ChatMessage &msg);
    void updateStreaming(const QString &delta);
    void endStreaming();
    void updateReasoning(const QString &delta);
    void showGreeting();
    QString lastAssistantHtml() const;
    void clear();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void copyRequested(const QString &id, const QString &content);
    void regenerateRequested(const QString &id);
    void editRequested(const QString &id);
    void deleteRequested(const QString &id);
    void continueRequested(const QString &id);

private:
    void clearLayout();
    void showActionsFor(QLabel *label, bool visible);
    void updateUserRowWidth(QWidget *row);

    QWidget *m_container;
    QVBoxLayout *m_layout;
    QVBoxLayout *m_rows;
    QList<QWidget *> m_userRows;
    QLabel *m_streamingLabel = nullptr;
    QGraphicsOpacityEffect *m_streamPulse = nullptr;
    QPropertyAnimation *m_streamPulseAnim = nullptr;
    QLabel *m_streamingReasoning = nullptr;
    QPushButton *m_reasoningToggle = nullptr;
    QString m_streamRaw;
    QString m_streamReasoning;
    QString m_streamId;
    QList<QString> m_codeBlocks;
    bool m_streamReasoningVisible = true;
};
