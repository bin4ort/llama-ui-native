#include "Api.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

Api::Api(QObject *parent) : QObject(parent), m_nam(new QNetworkAccessManager(this)) {}

void Api::setBaseUrl(const QString &url) { m_base = url; }
QString Api::baseUrl() const { return m_base; }

void Api::fetchModels() {
    QNetworkRequest req{QUrl(m_base + QStringLiteral("/v1/models"))};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    auto *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(QStringLiteral("LLMUI-API-003"), reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        emit modelsLoaded(doc.object().value(QStringLiteral("data")).toArray());
    });
}

void Api::startCompletion(const QJsonArray &messages, const QString &model,
                          double temperature) {
    if (m_stream) {
        m_stream->abort();
        m_stream = nullptr;
    }
    QJsonObject body{
        {QStringLiteral("model"), model},
        {QStringLiteral("messages"), messages},
        {QStringLiteral("temperature"), temperature},
        {QStringLiteral("stream"), true},
    };
    QNetworkRequest req{QUrl(m_base + QStringLiteral("/v1/chat/completions"))};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    m_stream = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    m_sseBuffer.clear();
    connect(m_stream, &QNetworkReply::readyRead, this, [this]() {
        m_sseBuffer += m_stream->readAll();
        // SSE lines: data: {…}\n\n
        while (true) {
            const int sep = m_sseBuffer.indexOf("\n\n");
            if (sep < 0) break;
            const QByteArray block = m_sseBuffer.left(sep);
            m_sseBuffer.remove(0, sep + 2);
            for (const auto &line : block.split('\n')) {
                if (!line.startsWith("data:")) continue;
                const QByteArray payload = line.mid(5).trimmed();
                if (payload == "[DONE]") {
                    emit contentDone();
                    continue;
                }
                const auto doc = QJsonDocument::fromJson(payload);
                const auto delta = doc.object()
                    .value(QStringLiteral("choices"))
                    .toArray().first().toObject()
                    .value(QStringLiteral("delta")).toObject()
                    .value(QStringLiteral("content")).toString();
                if (!delta.isEmpty()) emit chunkReceived(delta);
            }
        }
    });
    connect(m_stream, &QNetworkReply::finished, this, [this]() {
        auto *r = m_stream;
        m_stream = nullptr;
        r->deleteLater();
        if (r->error() != QNetworkReply::NoError && r->error() != QNetworkReply::OperationCanceledError)
            emit requestFailed(QStringLiteral("LLMUI-STR-000"), r->errorString());
        else
            emit contentDone();
    });
}

void Api::complete(const QJsonArray &messages, const QString &model,
                   double temperature, int maxTokens) {
    QJsonObject body{
        {QStringLiteral("model"), model},
        {QStringLiteral("messages"), messages},
        {QStringLiteral("temperature"), temperature},
        {QStringLiteral("max_tokens"), maxTokens},
        {QStringLiteral("stream"), false},
    };
    QNetworkRequest req{QUrl(m_base + QStringLiteral("/v1/chat/completions"))};
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    auto *reply = m_nam->post(req, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit requestFailed(QStringLiteral("LLMUI-API-018"), reply->errorString());
            return;
        }
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        const auto content = doc.object()
            .value(QStringLiteral("choices")).toArray().first().toObject()
            .value(QStringLiteral("message")).toObject()
            .value(QStringLiteral("content")).toString();
        emit completionReady(content);
    });
}

void Api::abort() {
    if (m_stream) {
        m_stream->abort();
        m_stream = nullptr;
    }
}
