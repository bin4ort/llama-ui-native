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
                          double temperature, const QJsonArray &tools) {
    if (m_stream) {
        m_stream->abort();
        m_stream = nullptr;
    }
    m_toolAccum = QJsonArray();
    QJsonObject body{
        {QStringLiteral("model"), model},
        {QStringLiteral("messages"), messages},
        {QStringLiteral("temperature"), temperature},
        {QStringLiteral("stream"), true},
    };
    if (!tools.isEmpty()) {
        body.insert(QStringLiteral("tools"), tools);
        body.insert(QStringLiteral("tool_choice"), QStringLiteral("auto"));
    }
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
                    continue; // stream end is signalled by the finished handler
                }
                const auto doc = QJsonDocument::fromJson(payload);
                const auto deltaObj = doc.object()
                    .value(QStringLiteral("choices"))
                    .toArray().first().toObject()
                    .value(QStringLiteral("delta")).toObject();
                const QString delta = deltaObj.value(QStringLiteral("content")).toString();
                if (!delta.isEmpty()) emit chunkReceived(delta);
                // reasoning deltas (Qwen thinking models): shown in a
                // collapsible "Reasoning" block (v2 §3.6)
                const QString reasoning =
                    deltaObj.value(QStringLiteral("reasoning_content")).toString();
                if (!reasoning.isEmpty()) emit reasoningReceived(reasoning);
                // streaming tool calls: accumulate by index
                const auto calls = deltaObj.value(QStringLiteral("tool_calls")).toArray();
                for (const auto &c : calls) {
                    const auto co = c.toObject();
                    const int idx = co.value(QStringLiteral("index")).toInt(0);
                    while (m_toolAccum.size() <= idx)
                        m_toolAccum.append(QJsonObject{
                            {QStringLiteral("id"), QString()},
                            {QStringLiteral("name"), QString()},
                            {QStringLiteral("arguments"), QString()}});
                    QJsonObject acc = m_toolAccum.at(idx).toObject();
                    const QString id = co.value(QStringLiteral("id")).toString();
                    if (!id.isEmpty()) acc.insert(QStringLiteral("id"), id);
                    const auto fn = co.value(QStringLiteral("function")).toObject();
                    const QString name = fn.value(QStringLiteral("name")).toString();
                    if (!name.isEmpty()) acc.insert(QStringLiteral("name"), name);
                    const QString args = fn.value(QStringLiteral("arguments")).toString();
                    if (!args.isEmpty())
                        acc.insert(QStringLiteral("arguments"),
                                   acc.value(QStringLiteral("arguments")).toString() + args);
                    m_toolAccum.replace(idx, acc);
                }
            }
        }
    });
    connect(m_stream, &QNetworkReply::finished, this, [this]() {
        auto *r = m_stream;
        m_stream = nullptr;
        r->deleteLater();
        if (r->error() != QNetworkReply::NoError && r->error() != QNetworkReply::OperationCanceledError)
            emit requestFailed(QStringLiteral("LLMUI-STR-000"), r->errorString());
        else {
            if (!m_toolAccum.isEmpty())
                emit toolCallsReady(m_toolAccum);
            emit contentDone();
        }
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

void Api::checkHealth() {
    QNetworkRequest req{QUrl(m_base + QStringLiteral("/health"))};
    auto *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        const bool ok = reply->error() == QNetworkReply::NoError &&
                        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200;
        emit healthChanged(ok);
    });
}

void Api::fetchProps() {
    QNetworkRequest req{QUrl(m_base + QStringLiteral("/props"))};
    auto *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) return;
        const auto doc = QJsonDocument::fromJson(reply->readAll());
        const auto defs = doc.object().value(QStringLiteral("default_generation_settings")).toObject();
        const int nCtx = defs.value(QStringLiteral("n_ctx")).toInt(4096);
        // the generation params live under 'params' (llama.cpp schema)
        QJsonObject merged = defs;
        const auto params = defs.value(QStringLiteral("params")).toObject();
        for (auto it = params.begin(); it != params.end(); ++it)
            merged.insert(it.key(), it.value());
        emit propsLoaded(qMax(512, nCtx), merged);
    });
}
