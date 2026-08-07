#include "MainWindow.h"
#include "Api.h"
#include "Config.h"
#include "Presets.h"
#include "ConversationStore.h"
#include "Theme.h"
#include "i18n.h"
#include "widgets/Composer.h"
#include "widgets/MessageView.h"
#include "widgets/SettingsDialog.h"
#include "widgets/WizardDialog.h"
#include "widgets/Sidebar.h"
#include <QApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineEdit>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("Llama UI Native"));
    resize(1440, 900);

    m_config = Config::load();
    I18n::init();
    I18n::setLanguage(m_config.value(QStringLiteral("language")).toString());

    m_api = new Api(this);
    m_api->setBaseUrl(m_config.value(QStringLiteral("serverEndpoint")).toString());
    m_store = new ConversationStore(this);
    m_store->load();

    auto *central = new QWidget(this);
    auto *layout = new QHBoxLayout(central);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_sidebar = new Sidebar(m_store, central);
    layout->addWidget(m_sidebar);

    auto *chatCol = new QVBoxLayout;
    chatCol->setContentsMargins(0, 0, 0, 0);
    chatCol->setSpacing(8);

    // header bar: model selector (spec §11)
    m_modelCombo = new QComboBox(central);
    m_modelCombo->setMinimumWidth(240);
    m_modelCombo->setMaximumWidth(420);
    chatCol->addWidget(m_modelCombo, 0, Qt::AlignLeft);

    m_messages = new MessageView(central);
    chatCol->addWidget(m_messages, 1);

    m_composer = new Composer(central);
    chatCol->addWidget(m_composer, 0, Qt::AlignHCenter);

    layout->addLayout(chatCol, 1);
    setCentralWidget(central);

    connect(m_sidebar, &Sidebar::newChatClicked, this, &MainWindow::newChat);
    connect(m_sidebar, &Sidebar::settingsClicked, this, &MainWindow::openSettings);
    connect(m_sidebar, &Sidebar::conversationSelected, this, &MainWindow::selectConversation);
    connect(m_composer, &Composer::sendRequested, this, &MainWindow::onSend);
    connect(m_composer, &Composer::personaRequested, this, &MainWindow::openPersonaMenu);
    connect(m_sidebar, &Sidebar::searchClicked, this, &MainWindow::openSearch);
    connect(m_sidebar, &Sidebar::mcpClicked, this, &MainWindow::openMcp);


    // streaming
    connect(m_api, &Api::chunkReceived, this, [this](const QString &delta) {
        m_messages->updateStreaming(delta);
        if (m_store->active() && !m_store->active()->messages.isEmpty()) {
            auto &last = m_store->active()->messages.last();
            last.content += delta;
        }
    });
    connect(m_api, &Api::contentDone, this, [this]() {
        m_streaming = false;
        m_composer->setBusy(false);
        if (m_store->active()) m_store->save();
        m_sidebar->refreshConversations();
    });
    connect(m_api, &Api::requestFailed, this, [this](const QString &code, const QString &msg) {
        m_streaming = false;
        m_composer->setBusy(false);
        QMessageBox::warning(this, QStringLiteral("Llama UI Native"),
                             QStringLiteral("Error %1 — %2").arg(code, msg));
    });

    // models
    connect(m_api, &Api::modelsLoaded, this, [this](const QJsonArray &models) {
        m_modelCombo->clear();
        for (const auto &m : models)
            m_modelCombo->addItem(m.toObject().value(QStringLiteral("id")).toString());
        if (!models.isEmpty()) {
            m_model = models.first().toObject().value(QStringLiteral("id")).toString();
            m_modelCombo->setCurrentIndex(0);
        }
    });
    connect(m_modelCombo, &QComboBox::currentTextChanged, this, [this](const QString &t) {
        m_model = t;
    });
    m_api->fetchModels();

    // restore last conversation if any
    if (!m_store->conversations().isEmpty()) {
        m_store->setActive(&m_store->conversations().first());
        selectConversation(m_store->active()->id);
    }

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

void MainWindow::newChat() {
    m_store->create(QString());
    m_messages->clear();
    refreshTitle();
    m_sidebar->refreshConversations();
}

void MainWindow::selectConversation(const QString &id) {
    for (auto &c : m_store->conversations()) {
        if (c.id == id) {
            m_store->setActive(&c);
            m_messages->clear();
            if (c.persona.isEmpty() && c.messages.isEmpty()) {
                m_messages->showGreeting();
            } else {
                for (const auto &m : c.messages) {
                    ChatMessage cm;
                    cm.role = m.role == QStringLiteral("user")    ? ChatMessage::User
                              : m.role == QStringLiteral("system") ? ChatMessage::System
                                                                   : ChatMessage::Assistant;
                    cm.content = m.content;
                    m_messages->addMessage(cm);
                }
            }
            refreshTitle();
            return;
        }
    }
}

void MainWindow::refreshTitle() {
    setWindowTitle(m_store->active() && !m_store->active()->name.isEmpty()
                       ? m_store->active()->name
                       : QStringLiteral("Llama UI Native"));
}

void MainWindow::onSend(const QString &text) {
    sendMessage(text);
}

void MainWindow::sendMessage(const QString &text) {
    if (m_streaming) return;
    if (!m_store->active()) newChat();
    auto *conv = m_store->active();
    m_streaming = true;
    m_composer->setBusy(true);

    ConvMsg user{uid(), QStringLiteral("user"), QStringLiteral("text"), text};
    m_store->appendMessage(conv, user);
    ConvMsg assistant{uid(), QStringLiteral("assistant"), QStringLiteral("text"), QString()};
    m_store->appendMessage(conv, assistant);

    ChatMessage cm{ChatMessage::User, text, false};
    m_messages->addMessage(cm);
    m_composer->clear();
    ChatMessage ph{ChatMessage::Assistant, QString(), true};
    m_messages->addMessage(ph);

    QJsonArray messages;
    if (!conv->persona.isEmpty())
        messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
                                     {QStringLiteral("content"), conv->persona}});
    for (const auto &m : conv->messages) {
        if (m.role == QStringLiteral("assistant") && m.content.isEmpty()) continue;
        if (m.role == QStringLiteral("system")) continue;
        messages.append(QJsonObject{{QStringLiteral("role"), m.role},
                                     {QStringLiteral("content"), m.content}});
    }
    m_api->startCompletion(messages, m_model);
}

void MainWindow::expandSidebarForTest() {
    m_sidebar->expandForTest();
}

void MainWindow::overrideThemeForTest(const QString &id) {
    applyTheme(id);
}

void MainWindow::openSettingsForTest() {
    openSettings();
}

void MainWindow::runSelfTest(const QString &message, const QString &shotPath) {
    newChat();
    auto *conn = new QMetaObject::Connection;
    *conn = connect(m_api, &Api::contentDone, this, [this, conn, shotPath]() {
        QObject::disconnect(*conn);
        delete conn;
        {
            QFile dbg(QStringLiteral("/tmp/opencode/v4-html-dump.txt"));
            if (dbg.open(QIODevice::WriteOnly))
                dbg.write(m_messages->lastAssistantHtml().toUtf8());
        }
        const auto img = grab().toImage().scaledToWidth(420, Qt::SmoothTransformation);
        img.save(shotPath, "JPG", 65);
        emit selfTestDone();
    });
    sendMessage(message);
}


void MainWindow::openSearch() {
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("appDialog"));
    dlg->setWindowTitle(QStringLiteral("Search"));
    dlg->setModal(true);
    dlg->resize(480, 560);
    auto *lay = new QVBoxLayout(dlg);
    auto *input = new QLineEdit(dlg);
    input->setPlaceholderText(QStringLiteral("Search conversations…"));
    lay->addWidget(input);
    auto *results = new QLabel(QStringLiteral("Type to search your conversations."), dlg);
    results->setWordWrap(true);
    lay->addWidget(results);
    auto *closeBtn = new QPushButton(QStringLiteral("Close"), dlg);
    closeBtn->setProperty("outline", true);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);
    connect(input, &QLineEdit::textChanged, dlg, [results, this](const QString &q) {
        const QString needle = q.trimmed();
        if (needle.isEmpty()) {
            results->setText(QStringLiteral("Type to search your conversations."));
            return;
        }
        QStringList hits;
        for (const auto &c : m_store->conversations())
            if (c.name.contains(needle, Qt::CaseInsensitive)) hits << c.name;
        results->setText(hits.isEmpty() ? QStringLiteral("No matches.")
                                        : QStringLiteral("Found:\n") + hits.join(QStringLiteral("\n")));
    });
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::openMcp() {
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("appDialog"));
    dlg->setWindowTitle(QStringLiteral("MCP Servers"));
    dlg->setModal(true);
    dlg->resize(480, 320);
    auto *lay = new QVBoxLayout(dlg);
    auto *info = new QLabel(
        QStringLiteral("MCP servers are configured in Settings (Tools). The server list will appear here once MCP support lands in this build."),
        dlg);
    info->setWordWrap(true);
    lay->addWidget(info);
    auto *closeBtn = new QPushButton(QStringLiteral("Close"), dlg);
    closeBtn->setProperty("outline", true);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);
    dlg->exec();
    dlg->deleteLater();
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

void MainWindow::openPersonaMenu() {
    auto *menu = new QMenu(this);
    const auto raw = Presets::load(m_config);
    for (const auto &p : Presets::list(raw)) {
        menu->addAction(p.name, this, [this, p]() {
            auto *conv = m_store->active();
            if (!conv) return;
            conv->persona = p.content;
            m_store->save();
            // persona row shown in the chat
            ConvMsg pm{uid(), QStringLiteral("system"), QStringLiteral("persona"), p.content};
            m_store->appendMessage(conv, pm);
            ChatMessage cm{ChatMessage::System, p.content, false};
            m_messages->addMessage(cm);
        });
    }
    menu->addSeparator();
    menu->addAction(QStringLiteral("Create with wizard…"), this, [this]() {
        WizardDialog dlg(m_api, m_model, this);
        connect(&dlg, &WizardDialog::presetCreated, this, [this](const QString &name,
                                                                  const QString &desc,
                                                                  const QString &content) {
            auto raw = Presets::load(m_config);
            m_config.insert(QStringLiteral("systemPromptPresets"),
                            Presets::add(raw, name, desc, content));
            Config::save(m_config);
        });
        dlg.exec();
    });
    menu->exec(QCursor::pos());
}
