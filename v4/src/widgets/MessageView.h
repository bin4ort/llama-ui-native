/*
 * MessageView.h — spec message rendering (UI-SPEC §3.3–3.5): user bubble
 * (primary/5, rounded 18px), assistant full-width text, persona/system
 * dashed bubble.
 */
#pragma once
#include <QScrollArea>
#include <QString>
#include <QVector>
class QLabel;

struct ChatMessage {
    enum Role { User, Assistant, System } role = Assistant;
    QString content;
    bool streaming = false;
};

class QVBoxLayout;
class QLabel;

class MessageView : public QScrollArea {
    Q_OBJECT
public:
    explicit MessageView(QWidget *parent = nullptr);

    void addMessage(const ChatMessage &msg);
    void updateStreaming(const QString &delta);
    void showGreeting();
    QString lastAssistantHtml() const;
    void clear();

private:
    QWidget *m_container;
    QVBoxLayout *m_layout;
    QLabel *m_streamingLabel = nullptr;
    QString m_streamRaw;
};
