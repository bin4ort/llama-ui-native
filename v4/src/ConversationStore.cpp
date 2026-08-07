#include "ConversationStore.h"
#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>

QString uid() {
    return QString::fromLatin1(
        QCryptographicHash::hash(QByteArray::number(quintptr(&uid)) +
                                     QByteArray::number(QDateTime::currentMSecsSinceEpoch()),
                                 QCryptographicHash::Md5)
            .toHex()
            .left(12));
}

QString ConversationStore::storagePath() const {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return base + QStringLiteral("/conversations.json");
}

void ConversationStore::load() {
    QFile f(storagePath());
    if (!f.open(QIODevice::ReadOnly)) return;
    const auto doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray()) return;
    for (const auto &v : doc.array()) {
        const auto obj = v.toObject();
        Conversation c;
        c.id = obj.value(QStringLiteral("id")).toString();
        c.name = obj.value(QStringLiteral("name")).toString();
        c.persona = obj.value(QStringLiteral("persona")).toString();
        for (const auto &mv : obj.value(QStringLiteral("messages")).toArray()) {
            const auto mo = mv.toObject();
            ConvMsg m;
            m.id = mo.value(QStringLiteral("id")).toString();
            m.role = mo.value(QStringLiteral("role")).toString();
            m.type = mo.value(QStringLiteral("type")).toString();
            m.content = mo.value(QStringLiteral("content")).toString();
            c.messages.append(m);
        }
        m_convs.append(c);
    }
}

void ConversationStore::save() const {
    QJsonArray arr;
    for (const auto &c : m_convs) {
        QJsonArray msgs;
        for (const auto &m : c.messages)
            msgs.append(QJsonObject{{QStringLiteral("id"), m.id},
                                    {QStringLiteral("role"), m.role},
                                    {QStringLiteral("type"), m.type},
                                    {QStringLiteral("content"), m.content}});
        arr.append(QJsonObject{{QStringLiteral("id"), c.id},
                               {QStringLiteral("name"), c.name},
                               {QStringLiteral("persona"), c.persona},
                               {QStringLiteral("messages"), msgs}});
    }
    QFile f(storagePath());
    if (f.open(QIODevice::WriteOnly))
        f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
}

Conversation *ConversationStore::create(const QString &name) {
    Conversation c;
    c.id = uid();
    c.name = name.isEmpty() ? QStringLiteral("New chat") : name;
    m_convs.prepend(c);
    m_active = &m_convs.first();
    return m_active;
}

void ConversationStore::remove(Conversation *c) {
    for (int i = 0; i < m_convs.size(); ++i) {
        if (&m_convs[i] == c) {
            m_convs.removeAt(i);
            m_active = m_convs.isEmpty() ? nullptr : &m_convs.first();
            return;
        }
    }
}

void ConversationStore::appendMessage(Conversation *c, const ConvMsg &msg) {
    c->messages.append(msg);
    save();
}
