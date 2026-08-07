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

    /** Start a streaming completion; emits chunkReceived/contentDone. */
    void startCompletion(const QJsonArray &messages, const QString &model,
                         double temperature = 0.7);

    void abort();

signals:
    void modelsLoaded(const QJsonArray &models);
    void requestFailed(const QString &code, const QString &message);
    void chunkReceived(const QString &delta);
    void contentDone();

private:
    QString m_base;
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_stream = nullptr;
    QByteArray m_sseBuffer;
};
