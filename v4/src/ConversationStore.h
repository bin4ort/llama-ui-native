/*
 * ConversationStore.h — conversation + message persistence (JSON file,
 * same tree model as the web app: persona/system/user/assistant rows).
 */
#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>
#include <QObject>

struct ConvMsg {
    QString id;
    QString role;   // user / assistant / system
    QString type;   // text / persona / system
    QString content;
    QJsonArray attachments; // [{name, mime, data(base64), image: bool}]
};

struct Conversation {
    QString id;
    QString name;
    QString persona;      // system prompt (persona row)
    bool pinned = false;
    QList<ConvMsg> messages;
};

QString uid();

class ConversationStore : public QObject {
    Q_OBJECT
public:
    explicit ConversationStore(QObject *parent = nullptr) : QObject(parent) {}
    void load();
    void save() const;

    QList<Conversation> &conversations() { return m_convs; }
    const QList<Conversation> &conversations() const { return m_convs; }

    Conversation *active() { return m_active; }
    void setActive(Conversation *c) { m_active = c; }

    Conversation *create(const QString &name);
    void remove(Conversation *c);
    void appendMessage(Conversation *c, const ConvMsg &msg);
    void rename(Conversation *c, const QString &name);

    QString storagePath() const;

private:
    QList<Conversation> m_convs;
    Conversation *m_active = nullptr;
};
