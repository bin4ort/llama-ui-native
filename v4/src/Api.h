/*
 * Api.h — minimal HTTP client for the local llama.cpp server
 * (models list + streaming chat completions). QNetworkAccessManager only —
 * no browser engine, no environment variables.
 */
#pragma once
#include <QJsonArray>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class Api : public QObject {
    Q_OBJECT
public:
    explicit Api(QObject *parent = nullptr);

    void setBaseUrl(const QString &url);
    QString baseUrl() const;

    /** Fetch the model list; emits modelsLoaded/requestFailed. */
    void fetchModels();

    /** Start a streaming completion; emits chunkReceived/reasoningReceived/
     *  contentDone/toolCallsReady. tools: OpenAI function schemas. */
    void startCompletion(const QJsonArray &messages, const QString &model,
                         double temperature = 0.7,
                         const QJsonArray &tools = QJsonArray());

    /** One-shot completion; emits completionReady with the full content. */
    void complete(const QJsonArray &messages, const QString &model,
                  double temperature = 0.4, int maxTokens = 1200);

    void abort();

    /** Poll server health; emits healthChanged on every answer. */
    void checkHealth();

    /** Fetch server props (context size); emits propsLoaded. */
    void fetchProps();

signals:
    void modelsLoaded(const QJsonArray &models);
    void requestFailed(const QString &code, const QString &message);
    void chunkReceived(const QString &delta);
    void reasoningReceived(const QString &delta);
    void toolCallsReady(const QJsonArray &toolCalls); // [{id,name,arguments}]
    void contentDone();
    void completionReady(const QString &content);
    void healthChanged(bool online);
    void propsLoaded(int nCtx);

private:
    QString m_base;
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_stream = nullptr;
    QByteArray m_sseBuffer;
    QJsonArray m_toolAccum; // streaming tool_calls accumulated by index
};
