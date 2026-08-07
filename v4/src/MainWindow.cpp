#include "MainWindow.h"
#include "Api.h"
#include "Config.h"
#include "Theme.h"
#include "i18n.h"
#include "widgets/Composer.h"
#include "widgets/MessageView.h"
#include "widgets/SettingsDialog.h"
#include "widgets/Sidebar.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Llama UI Native"));
    resize(1440, 900);

    m_config = Config::load();
    I18n::init();
    I18n::setLanguage(m_config.value(QStringLiteral("language")).toString());

    m_api = new Api(this);
    m_api->setBaseUrl(m_config.value(QStringLiteral("serverEndpoint")).toString());

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_sidebar = new Sidebar(central);
    layout->addWidget(m_sidebar);

    auto *chatCol = new QVBoxLayout;
    chatCol->setContentsMargins(0, 0, 0, 0);
    chatCol->setSpacing(8);

    m_messages = new MessageView(central);
    chatCol->addWidget(m_messages, 1);

    m_composer = new Composer(central);
    chatCol->addWidget(m_composer, 0, Qt::AlignHCenter);

    layout->addLayout(chatCol, 1);
    setCentralWidget(central);

    connect(m_sidebar, &Sidebar::newChatClicked, this, [this]() {
        m_messages->clear();
    });
    connect(m_sidebar, &Sidebar::settingsClicked, this, &MainWindow::openSettings);
    connect(m_composer, &Composer::sendRequested, this, &MainWindow::onSend);

    // streaming
    connect(m_api, &Api::chunkReceived, this, [this](const QString &delta) {
        m_messages->updateStreaming(delta);
    });
    connect(m_api, &Api::contentDone, this, [this]() {
        m_streaming = false;
        m_composer->setBusy(false);
    });
    connect(m_api, &Api::requestFailed, this, [this](const QString &code, const QString &msg) {
        m_streaming = false;
        m_composer->setBusy(false);
        QMessageBox::warning(this, QStringLiteral("Llama UI Native"),
                             QStringLiteral("Error %1 — %2").arg(code, msg));
    });

    // models
    connect(m_api, &Api::modelsLoaded, this, [this](const QJsonArray &models) {
        if (!models.isEmpty())
            m_model = models.first().toObject().value(QStringLiteral("id")).toString();
    });
    m_api->fetchModels();

    applyTheme(m_config.value(QStringLiteral("theme")).toString());
}

void MainWindow::applyTheme(const QString &paletteId) {
    qApp->setStyleSheet(Theme::stylesheet(paletteId));
    const auto t = Theme::tokens(paletteId);
    QPalette pal;
    pal.setColor(QPalette::Window, t.background);
    pal.setColor(QPalette::WindowText, t.foreground);
    pal.setColor(QPalette::Base, t.card);
    pal.setColor(QPalette::Text, t.foreground);
    qApp->setPalette(pal);
}

void MainWindow::onSend(const QString &text) {
    sendMessage(text, m_config.value(QStringLiteral("systemMessage")).toString());
}

void MainWindow::sendMessage(const QString &text, const QString &systemMessage) {
    if (m_streaming) return;
    m_streaming = true;
    m_composer->setBusy(true);

    ChatMessage user{ChatMessage::User, text, false};
    m_messages->addMessage(user);
    m_composer->clear();

    ChatMessage placeholder{ChatMessage::Assistant, QString(), true};
    m_messages->addMessage(placeholder);

    QJsonArray messages;
    if (!systemMessage.isEmpty())
        messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
                                     {QStringLiteral("content"), systemMessage}});
    messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("user")},
                                {QStringLiteral("content"), text}});
    m_api->startCompletion(messages, m_model);
}

void MainWindow::openSettings() {
    SettingsDialog dlg(m_config, this);
    connect(&dlg, &SettingsDialog::configChanged, this, [this](const QJsonObject &cfg) {
        m_config = cfg;
        Config::save(cfg);
        I18n::setLanguage(cfg.value(QStringLiteral("language")).toString());
        m_api->setBaseUrl(cfg.value(QStringLiteral("serverEndpoint")).toString());
        applyTheme(cfg.value(QStringLiteral("theme")).toString());
    });
    dlg.exec();
}
