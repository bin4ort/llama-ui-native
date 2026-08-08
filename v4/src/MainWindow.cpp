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
#include <QBuffer>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QWidget>

/** Encode a file for the multimodal payload (UI-SPEC §4): images become
 *  base64 data URLs, other files keep their raw name (text extraction
 *  would need per-type parsing — the model sees images natively). */
static QJsonObject encodeAttachment(const QString &path) {
    const QString lower = path.toLower();
    const bool image = lower.endsWith(QStringLiteral(".png")) ||
                       lower.endsWith(QStringLiteral(".jpg")) ||
                       lower.endsWith(QStringLiteral(".jpeg")) ||
                       lower.endsWith(QStringLiteral(".webp")) ||
                       lower.endsWith(QStringLiteral(".gif"));
    QJsonObject att;
    att.insert(QStringLiteral("name"), QFileInfo(path).fileName());
    att.insert(QStringLiteral("mime"), QString());
    att.insert(QStringLiteral("image"), image);
    if (image) {
        QImage img(path);
        if (img.isNull()) {
            att.insert(QStringLiteral("error"), true);
            return att;
        }
        // downscale to <=1.5 MP (maxImageMPixels-ish default)
        if (img.width() * img.height() > 1500000)
            img = img.scaled(1600, 1600, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        img.save(&buffer, "PNG");
        att.insert(QStringLiteral("data"),
                   QString::fromLatin1(bytes.toBase64()));
    } else {
        QFile f(path);
        if (f.open(QIODevice::ReadOnly)) {
            att.insert(QStringLiteral("data"),
                       QString::fromLatin1(f.readAll().toBase64()));
        }
    }
    return att;
}

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

    // server error banner (UI-SPEC §6.6) — replaces the old modal nag
    auto *bannerHost = new QWidget(central);
    bannerHost->setObjectName(QStringLiteral("bannerHost"));
    auto *bl = new QHBoxLayout(bannerHost);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);
    m_errorBanner = new QLabel(bannerHost);
    m_errorBanner->setObjectName(QStringLiteral("errorBanner"));
    m_errorBanner->setWordWrap(true);
    bl->addWidget(m_errorBanner, 1);
    auto *retryBtn = new QPushButton(I18n::t(QStringLiteral("Retry")), bannerHost);
    retryBtn->setProperty("outline", true);
    connect(retryBtn, &QPushButton::clicked, this, [this]() {
        hideErrorBanner();
        m_api->fetchModels();
        if (m_store->active()) m_api->checkHealth();
    });
    bl->addWidget(retryBtn);
    bannerHost->setVisible(false);
    chatCol->addWidget(bannerHost);

    m_messages = new MessageView(central);
    chatCol->addWidget(m_messages, 1);

    m_composer = new Composer(central);
    chatCol->addWidget(m_composer, 0, Qt::AlignHCenter);

    layout->addLayout(chatCol, 1);
    setCentralWidget(central);

    // sidebar wiring
    connect(m_sidebar, &Sidebar::newChatClicked, this, &MainWindow::newChat);
    connect(m_sidebar, &Sidebar::settingsClicked, this, &MainWindow::openSettings);
    connect(m_sidebar, &Sidebar::conversationSelected, this, &MainWindow::selectConversation);
    connect(m_sidebar, &Sidebar::conversationRenameRequested, this, &MainWindow::onRenameConversation);
    connect(m_sidebar, &Sidebar::conversationDeleteRequested, this, &MainWindow::onDeleteConversation);
    connect(m_sidebar, &Sidebar::searchClicked, this, &MainWindow::openSearch);
    connect(m_sidebar, &Sidebar::mcpClicked, this, &MainWindow::openMcp);

    // composer wiring
    connect(m_composer, &Composer::sendRequested, this, &MainWindow::onSend);
    connect(m_composer, &Composer::stopRequested, this, [this]() {
        m_api->abort();
        m_composer->setBusy(false);
    });
    connect(m_composer, &Composer::personaRequested, this, &MainWindow::openPersonaMenu);
    connect(m_composer, &Composer::attachRequested, this, [this]() {
        const QStringList files = QFileDialog::getOpenFileNames(
            this, I18n::t(QStringLiteral("Attach file")),
            QString(), I18n::t(QStringLiteral("Images (*.png *.jpg *.jpeg *.webp *.gif);;All files (*)")));
        if (!files.isEmpty()) attachFiles(files);
    });
    setAcceptDrops(true);

    // message actions (UI-SPEC §3.9)
    connect(m_messages, &MessageView::copyRequested, this, &MainWindow::onCopy);
    connect(m_messages, &MessageView::regenerateRequested, this, &MainWindow::onRegenerate);
    connect(m_messages, &MessageView::editRequested, this, &MainWindow::onEdit);
    connect(m_messages, &MessageView::deleteRequested, this, &MainWindow::onDelete);
    connect(m_messages, &MessageView::suggestionClicked, this,
            [this](const QString &t) { sendMessageWithHistory(t); });

    // streaming
    connect(m_api, &Api::chunkReceived, this, [this](const QString &delta) {
        hideErrorBanner();
        m_messages->updateStreaming(delta);
        if (m_store->active() && !m_store->active()->messages.isEmpty()) {
            auto &last = m_store->active()->messages.last();
            last.content += delta;
        }
    });
    connect(m_api, &Api::reasoningReceived, this,
            [this](const QString &delta) { m_messages->updateReasoning(delta); });
    connect(m_api, &Api::contentDone, this, &MainWindow::onStreamFinished);
    connect(m_api, &Api::requestFailed, this, [this](const QString &code, const QString &msg) {
        m_streaming = false;
        m_composer->setBusy(false);
        showErrorBanner(QStringLiteral("LLMUI-") + code +
                        QStringLiteral(" — ") + msg);
    });

    // models
    connect(m_api, &Api::modelsLoaded, this, [this](const QJsonArray &models) {
        m_modelCombo->clear();
        for (const auto &m : models)
            m_modelCombo->addItem(m.toObject().value(QStringLiteral("id")).toString());
        if (!models.isEmpty()) {
            const QString preferred = Config::value(QStringLiteral("model")).toString();
            int found = m_modelCombo->findText(preferred);
            if (found < 0) found = 0;
            m_model = m_modelCombo->itemText(found);
            m_modelCombo->setCurrentIndex(found);
        }
    });
    connect(m_modelCombo, &QComboBox::currentTextChanged, this, [this](const QString &t) {
        m_model = t;
        if (!t.isEmpty()) {
            m_config.insert(QStringLiteral("model"), t);
            Config::save(m_config);
        }
    });
    m_api->fetchModels();

    // server status (UI-SPEC §2.5) — health poll
    connect(m_api, &Api::healthChanged, this, [this](bool ok) {
        setServerStatus(ok);
    });
    auto *healthTimer = new QTimer(this);
    connect(healthTimer, &QTimer::timeout, m_api, &Api::checkHealth);
    healthTimer->start(10000);
    m_api->checkHealth();

    // keyboard shortcuts (UI-SPEC §2.1): Ctrl+K search, Ctrl+B sidebar,
    // Ctrl+Shift+O new chat
    auto *kSearch = new QShortcut(QKeySequence(QStringLiteral("Ctrl+K")), this);
    connect(kSearch, &QShortcut::activated, this, &MainWindow::openSearch);
    auto *kSidebar = new QShortcut(QKeySequence(QStringLiteral("Ctrl+B")), this);
    connect(kSidebar, &QShortcut::activated, m_sidebar, &Sidebar::toggleExpanded);
    auto *kNew = new QShortcut(QKeySequence(QStringLiteral("Ctrl+Shift+O")), this);
    connect(kNew, &QShortcut::activated, this, &MainWindow::newChat);

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
    m_sidebar->setActiveConversation(m_store->active()->id);
    refreshTitle();
    m_sidebar->refreshConversations();
    m_composer->focusInput();
}

void MainWindow::selectConversation(const QString &id) {
    for (auto &c : m_store->conversations()) {
        if (c.id == id) {
            m_store->setActive(&c);
            m_sidebar->setActiveConversation(id);
            renderConversation(&c);
            refreshTitle();
            return;
        }
    }
}

void MainWindow::renderConversation(Conversation *conv) {
    m_messages->clear();
    if (conv->persona.isEmpty() && conv->messages.isEmpty()) {
        m_messages->showGreeting();
        return;
    }
    for (const auto &m : conv->messages) {
        ChatMessage cm;
        cm.id = m.id;
        cm.type = m.type;
        cm.role = m.role == QStringLiteral("user")    ? ChatMessage::User
                  : m.role == QStringLiteral("system") ? ChatMessage::System
                                                       : ChatMessage::Assistant;
        cm.content = m.content;
        cm.attachments = m.attachments;
        m_messages->addMessage(cm);
    }
}

void MainWindow::refreshTitle() {
    setWindowTitle(m_store->active() && !m_store->active()->name.isEmpty()
                       ? m_store->active()->name
                       : QStringLiteral("Llama UI Native"));
}

void MainWindow::onSend(const QString &text) {
    sendMessageWithHistory(text);
}

void MainWindow::attachFiles(const QStringList &paths) {
    for (const auto &p : paths)
        m_composer->addAttachment(encodeAttachment(p));
    m_composer->setAttachments(m_composer->attachments());
    m_composer->focusInput();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QStringList files;
    for (const auto &u : event->mimeData()->urls()) {
        if (u.isLocalFile()) files << u.toLocalFile();
    }
    if (!files.isEmpty()) attachFiles(files);
}

void MainWindow::sendMessageWithHistory(const QString &text) {
    if (m_streaming) return;
    if (!m_store->active()) newChat();
    auto *conv = m_store->active();
    m_streaming = true;
    m_composer->setBusy(true);
    hideErrorBanner();

    const QJsonArray atts = m_composer->attachments();
    m_composer->clear();

    // auto-title from first message (like the web app)
    if (conv->messages.isEmpty() && conv->name == QStringLiteral("New chat")) {
        conv->name = text.left(32) + (text.size() > 32 ? QStringLiteral("…") : QString());
        refreshTitle();
    }

    ConvMsg user{uid(), QStringLiteral("user"), QStringLiteral("text"), text, atts};
    m_store->appendMessage(conv, user);
    ConvMsg assistant{uid(), QStringLiteral("assistant"), QStringLiteral("text"), QString()};
    m_store->appendMessage(conv, assistant);

    ChatMessage cm{ChatMessage::User, user.id, text, QStringLiteral("text"), atts, false};
    m_messages->addMessage(cm);
    ChatMessage ph{ChatMessage::Assistant, assistant.id, QString(), QStringLiteral("text"), QJsonArray(), true};
    m_messages->addMessage(ph);

    QJsonArray messages;
    if (!conv->persona.isEmpty())
        messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
                                     {QStringLiteral("content"), conv->persona}});
    for (const auto &m : conv->messages) {
        if (m.role == QStringLiteral("assistant") && m.content.isEmpty()) continue;
        if (m.role == QStringLiteral("system")) continue;
        QJsonValue content = m.content;
        if (!m.attachments.isEmpty()) {
            // multimodal parts (OpenAI image_url format, UI-SPEC §4)
            QJsonArray parts;
            parts.append(QJsonObject{{QStringLiteral("type"), QStringLiteral("text")},
                                     {QStringLiteral("text"), m.content}});
            for (const auto &a : m.attachments) {
                const auto att = a.toObject();
                if (att.value(QStringLiteral("image")).toBool()) {
                    parts.append(QJsonObject{
                        {QStringLiteral("type"), QStringLiteral("image_url")},
                        {QStringLiteral("image_url"),
                         QJsonObject{{QStringLiteral("url"),
                                      QStringLiteral("data:image/png;base64,") +
                                          att.value(QStringLiteral("data")).toString()}}}});
                } else {
                    parts.append(QJsonObject{
                        {QStringLiteral("type"), QStringLiteral("text")},
                        {QStringLiteral("text"),
                         QStringLiteral("[attachment: ") +
                             att.value(QStringLiteral("name")).toString() +
                             QStringLiteral("]")}});
                }
            }
            content = parts;
        }
        messages.append(QJsonObject{{QStringLiteral("role"), m.role},
                                     {QStringLiteral("content"), content}});
    }
    m_api->startCompletion(messages, m_model);
}

void MainWindow::onStreamFinished() {
    m_streaming = false;
    m_composer->setBusy(false);
    if (m_store->active()) {
        // drop an empty assistant message left by a user stop
        auto &msgs = m_store->active()->messages;
        if (!msgs.isEmpty() && msgs.last().role == QStringLiteral("assistant") &&
            msgs.last().content.trimmed().isEmpty())
            msgs.removeLast();
        m_store->save();
    }
    m_sidebar->refreshConversations();
    m_api->checkHealth();
}

void MainWindow::showErrorBanner(const QString &text) {
    m_errorBanner->setText(text);
    m_errorBanner->parentWidget()->setVisible(true);
    setServerStatus(false);
}

void MainWindow::hideErrorBanner() {
    if (m_errorBanner) m_errorBanner->parentWidget()->setVisible(false);
}

void MainWindow::setServerStatus(bool online) {
    m_sidebar->setServerStatus(
        online, QUrl(m_api->baseUrl()).host() + QStringLiteral(":") +
                    QString::number(QUrl(m_api->baseUrl()).port(8080)));
}

void MainWindow::onCopy(const QString &, const QString &content) {
    QApplication::clipboard()->setText(content);
}

void MainWindow::onRegenerate(const QString &id) {
    auto *conv = m_store->active();
    if (!conv) return;
    int idx = -1;
    for (int i = 0; i < conv->messages.size(); ++i)
        if (conv->messages.at(i).id == id) { idx = i; break; }
    if (idx < 0) return;
    // truncate at the regenerated message; resend the preceding user prompt
    conv->messages = conv->messages.mid(0, idx);
    QString prompt;
    if (!conv->messages.isEmpty() &&
        conv->messages.last().role == QStringLiteral("user"))
        prompt = conv->messages.last().content;
    m_store->save();
    renderConversation(conv);
    if (!prompt.isEmpty()) sendMessageWithHistory(prompt);
}

void MainWindow::onEdit(const QString &id) {
    auto *conv = m_store->active();
    if (!conv) return;
    int idx = -1;
    for (int i = 0; i < conv->messages.size(); ++i)
        if (conv->messages.at(i).id == id) { idx = i; break; }
    if (idx < 0) return;
    const QString text = conv->messages.at(idx).content;
    // truncate: remove this message and everything after
    conv->messages = conv->messages.mid(0, idx);
    m_store->save();
    renderConversation(conv);
    m_composer->setTextForEdit(text);
    m_composer->focusInput();
}

void MainWindow::onDelete(const QString &id) {
    auto *conv = m_store->active();
    if (!conv) return;
    for (int i = 0; i < conv->messages.size(); ++i) {
        if (conv->messages.at(i).id == id) {
            conv->messages.removeAt(i);
            break;
        }
    }
    m_store->save();
    renderConversation(conv);
}

void MainWindow::onRenameConversation(const QString &id) {
    for (auto &c : m_store->conversations()) {
        if (c.id == id) {
            bool ok = false;
            const QString name =
                QInputDialog::getText(this, I18n::t(QStringLiteral("Rename conversation")),
                                      I18n::t(QStringLiteral("Name")), QLineEdit::Normal,
                                      c.name, &ok);
            if (ok && !name.trimmed().isEmpty()) {
                m_store->rename(&c, name.trimmed());
                m_sidebar->refreshConversations();
                refreshTitle();
            }
            return;
        }
    }
}

void MainWindow::onDeleteConversation(const QString &id) {
    auto *conv = m_store->active();
    if (!conv || conv->id != id) {
        for (auto &c : m_store->conversations())
            if (c.id == id) { conv = &c; break; }
    }
    if (!conv) return;
    const auto ret = QMessageBox::question(
        this, I18n::t(QStringLiteral("Delete conversation")),
        I18n::t(QStringLiteral("Are you sure you want to delete this conversation?")));
    if (ret != QMessageBox::Yes) return;
    m_store->remove(conv);
    m_sidebar->refreshConversations();
    if (m_store->active())
        selectConversation(m_store->active()->id);
    else {
        m_messages->clear();
        refreshTitle();
    }
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
    sendMessageWithHistory(message);
}

void MainWindow::openSearch() {
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("appDialog"));
    dlg->setWindowTitle(I18n::t(QStringLiteral("Search")));
    dlg->setModal(true);
    dlg->resize(480, 560);
    auto *lay = new QVBoxLayout(dlg);
    auto *input = new QLineEdit(dlg);
    input->setPlaceholderText(I18n::t(QStringLiteral("Search conversations and messages")));
    lay->addWidget(input);
    auto *results = new QLabel(I18n::t(QStringLiteral("Type to search your conversations.")), dlg);
    results->setWordWrap(true);
    lay->addWidget(results);
    auto *closeBtn = new QPushButton(I18n::t(QStringLiteral("Close")), dlg);
    closeBtn->setProperty("outline", true);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);
    connect(input, &QLineEdit::textChanged, dlg, [results, this](const QString &q) {
        const QString needle = q.trimmed();
        if (needle.isEmpty()) {
            results->setText(I18n::t(QStringLiteral("Type to search your conversations.")));
            return;
        }
        QStringList hits;
        int convHits = 0;
        int msgHits = 0;
        for (const auto &c : m_store->conversations()) {
            if (c.name.contains(needle, Qt::CaseInsensitive)) {
                ++convHits;
                hits << QStringLiteral("▸ ") + c.name;
            }
        }
        for (const auto &c : m_store->conversations()) {
            for (const auto &m : c.messages) {
                if (m.content.contains(needle, Qt::CaseInsensitive)) {
                    ++msgHits;
                    const QString snippet = m.content.mid(0, 90).replace('\n', ' ') +
                                            (m.content.size() > 90 ? QStringLiteral("…") : QString());
                    hits << QStringLiteral("  ↳ %1: “%2”").arg(c.name, snippet);
                }
            }
        }
        if (hits.isEmpty())
            results->setText(I18n::t(QStringLiteral("No matches.")));
        else
            results->setText(QStringLiteral("%1 conversations, %2 messages\n\n%3")
                                 .arg(convHits)
                                 .arg(msgHits)
                                 .arg(hits.join('\n')));
    });
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::openMcp() {
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("appDialog"));
    dlg->setWindowTitle(I18n::t(QStringLiteral("MCP Servers")));
    dlg->setModal(true);
    dlg->resize(480, 320);
    auto *lay = new QVBoxLayout(dlg);
    auto *info = new QLabel(
        I18n::t(QStringLiteral("MCP servers are configured in Settings (Tools). The server list will appear here once MCP support lands in this build.")),
        dlg);
    info->setWordWrap(true);
    lay->addWidget(info);
    auto *closeBtn = new QPushButton(I18n::t(QStringLiteral("Close")), dlg);
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
            ChatMessage cm{ChatMessage::System, pm.id, p.content, QStringLiteral("persona"),
                           QJsonArray(), false};
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
