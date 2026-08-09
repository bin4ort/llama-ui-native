#include "MainWindow.h"
#include "Api.h"
#include "Config.h"
#include "Presets.h"
#include "ConversationStore.h"
#include "Theme.h"
#include "i18n.h"
#include "Icons.h"
#include "widgets/Composer.h"
#include "widgets/MessageView.h"
#include "widgets/SettingsDialog.h"
#include "widgets/WizardDialog.h"
#include "widgets/Sidebar.h"
#include "widgets/McpPage.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDateTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEventLoop>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QShortcut>
#include <QTextEdit>
#include <QTimer>
#include <QToolButton>
#include <QUrl>
#include <QWidget>

namespace {

/** Recursive-descent evaluator for the calculator tool (safe subset:
 *  digits + + - * / ( ) . — no eval(), no strings). */
double parseSum(QStringView &s, bool *ok);

double parseAtom(QStringView &s, bool *ok) {
    s = s.trimmed();
    if (s.startsWith('(')) {
        s = s.mid(1);
        const double v = parseSum(s, ok);
        s = s.trimmed();
        if (!s.startsWith(')')) { *ok = false; return 0; }
        s = s.mid(1);
        return v;
    }
    int i = 0;
    bool dot = false;
    while (i < s.size()) {
        const QChar c = s.at(i);
        if (c.isDigit()) ++i;
        else if (c == '.' && !dot) { dot = true; ++i; }
        else break;
    }
    if (i == 0) { *ok = false; return 0; }
    const double v = s.left(i).toString().toDouble(ok);
    s = s.mid(i);
    return v;
}

double parseProd(QStringView &s, bool *ok) {
    double v = parseAtom(s, ok);
    while (*ok) {
        s = s.trimmed();
        if (s.startsWith('*')) { s = s.mid(1); v *= parseAtom(s, ok); }
        else if (s.startsWith('/')) { s = s.mid(1); v /= parseAtom(s, ok); }
        else break;
    }
    return v;
}

double parseSum(QStringView &s, bool *ok) {
    double v = parseProd(s, ok);
    while (*ok) {
        s = s.trimmed();
        if (s.startsWith('+')) { s = s.mid(1); v += parseProd(s, ok); }
        else if (s.startsWith('-')) { s = s.mid(1); v -= parseProd(s, ok); }
        else break;
    }
    return v;
}

double safeEval(const QString &expr, bool *ok) {
    *ok = true;
    QStringView s(expr);
    const double v = parseSum(s, ok);
    s = s.trimmed();
    if (!s.isEmpty()) *ok = false;
    return v;
}

/** Built-in tool schemas (v2 approved tools subset implemented natively). */
QJsonArray toolSchemas() {
    const auto fn = [](const QString &name, const QString &desc,
                       const QJsonObject &props, const QStringList &req) {
        return QJsonObject{{QStringLiteral("type"), QStringLiteral("function")},
                           {QStringLiteral("function"),
                            QJsonObject{
                                {QStringLiteral("name"), name},
                                {QStringLiteral("description"), desc},
                                {QStringLiteral("parameters"),
                                 QJsonObject{{QStringLiteral("type"), QStringLiteral("object")},
                                             {QStringLiteral("properties"), props},
                                             {QStringLiteral("required"), QJsonArray::fromStringList(req)}}}}}};
    };
    QJsonArray tools;
    tools.append(fn(QStringLiteral("get_datetime"),
                    QStringLiteral("Get the current date and time. Use this when the user asks what time or date it is."),
                    QJsonObject{{QStringLiteral("format"),
                                 QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                             {QStringLiteral("description"), QStringLiteral("optional strftime format")}}}},
                    {}));
    tools.append(fn(QStringLiteral("calculate"),
                    QStringLiteral("Evaluate a simple arithmetic expression (+, -, *, /, parentheses)."),
                    QJsonObject{{QStringLiteral("expression"),
                                 QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                             {QStringLiteral("description"), QStringLiteral("the expression to evaluate")}}}},
                    {QStringLiteral("expression")}));
    tools.append(fn(QStringLiteral("fetch_url"),
                    QStringLiteral("Fetch the text content of a URL (strips HTML)."),
                    QJsonObject{{QStringLiteral("url"),
                                 QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                             {QStringLiteral("description"), QStringLiteral("http(s) URL")}}}},
                    {QStringLiteral("url")}));
    return tools;
}

} // namespace

double v4SafeEval(const QString &expr, bool *ok) {
    *ok = true;
    QStringView s(expr);
    const double v = parseSum(s, ok);
    s = s.trimmed();
    if (!s.isEmpty()) *ok = false;
    return v;
}

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
    att.insert(QStringLiteral("size"), qint64(QFileInfo(path).size()));
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

    // server error banner (v2 ChatScreenServerError) — above the composer
    auto *bannerHost = new QWidget(central);
    bannerHost->setObjectName(QStringLiteral("bannerHost"));
    auto *bl = new QHBoxLayout(bannerHost);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(8);
    auto *warnIcon = new QLabel(QStringLiteral("⚠ "), bannerHost);
    warnIcon->setObjectName(QStringLiteral("bannerIcon"));
    bl->addWidget(warnIcon);
    auto *bannerText = new QVBoxLayout;
    bannerText->setContentsMargins(0, 0, 0, 0);
    bannerText->setSpacing(0);
    m_errorBanner = new QLabel(I18n::t(QStringLiteral("Server unavailable")), bannerHost);
    m_errorBanner->setObjectName(QStringLiteral("errorBanner"));
    bannerText->addWidget(m_errorBanner);
    m_errorBannerDetail = new QLabel(bannerHost);
    m_errorBannerDetail->setObjectName(QStringLiteral("errorBannerDetail"));
    m_errorBannerDetail->setWordWrap(true);
    bannerText->addWidget(m_errorBannerDetail);
    bl->addLayout(bannerText, 1);
    auto *retryBtn = new QPushButton(QStringLiteral("↻ ") + I18n::t(QStringLiteral("Retry")), bannerHost);
    retryBtn->setProperty("outline", true);
    connect(retryBtn, &QPushButton::clicked, this, [this]() {
        hideErrorBanner();
        m_api->fetchModels();
        if (m_store->active()) m_api->checkHealth();
    });
    bl->addWidget(retryBtn);
    bannerHost->setMaximumWidth(768);
    bannerHost->setVisible(false);

    m_messages = new MessageView(central);
    chatCol->addWidget(m_messages, 1);

    m_composer = new Composer(central);
    chatCol->addWidget(m_composer, 0, Qt::AlignHCenter);
    chatCol->insertWidget(chatCol->indexOf(m_composer), bannerHost, 0, Qt::AlignHCenter); // v2: banner above the form
    // v2 floats the form at ~50dvh on the empty chat (md:bottom-[calc(50dvh-7rem)])
    // and pins it to the bottom once messages exist
    m_formSpacer = new QWidget(central);
    m_formSpacer->setVisible(false);
    chatCol->addWidget(m_formSpacer);
    chatCol->setStretch(chatCol->indexOf(m_messages), 1);

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
    connect(m_composer, &Composer::modelSelected, this, [this](const QString &id) {
        m_model = id;
        if (!id.isEmpty()) {
            m_config.insert(QStringLiteral("model"), id);
            Config::save(m_config);
        }
    });
    connect(m_composer, &Composer::attachRequested, this, [this]() {
        // v2 ChatFormActionAddDropdown: Add files / System message
        QMenu menu(this);
        const auto filesSub = menu.addMenu(QStringLiteral("📎 ") +
                                           I18n::t(QStringLiteral("Add files")));
        filesSub->addAction(I18n::t(QStringLiteral("Images")),
                            this, [this]() {
                                const QStringList files = QFileDialog::getOpenFileNames(
                                    this, I18n::t(QStringLiteral("Images")),
                                    QString(),
                                    QStringLiteral("Images (*.png *.jpg *.jpeg *.webp *.gif)"));
                                if (!files.isEmpty()) attachFiles(files);
                            });
        filesSub->addAction(I18n::t(QStringLiteral("Text Files")),
                            this, [this]() {
                                const QStringList files = QFileDialog::getOpenFileNames(
                                    this, I18n::t(QStringLiteral("Text Files")),
                                    QString(), QStringLiteral("Text (*.txt *.md *.json *.csv *.log)"));
                                if (!files.isEmpty()) attachFiles(files);
                            });
        filesSub->addAction(I18n::t(QStringLiteral("PDF Files")),
                            this, [this]() {
                                const QStringList files = QFileDialog::getOpenFileNames(
                                    this, I18n::t(QStringLiteral("PDF Files")),
                                    QString(), QStringLiteral("PDF (*.pdf)"));
                                if (!files.isEmpty()) attachFiles(files);
                            });
        filesSub->addAction(I18n::t(QStringLiteral("All files")),
                            this, [this]() {
                                const QStringList files = QFileDialog::getOpenFileNames(
                                    this, I18n::t(QStringLiteral("Add files")),
                                    QString(), QStringLiteral("All files (*)"));
                                if (!files.isEmpty()) attachFiles(files);
                            });
        menu.addSeparator();
        // MCP servers submenu (v2: per-server switches + configure footer)
        const auto servers = McpServers::list(m_config);
        if (!servers.isEmpty()) {
            auto *mcpSub = menu.addMenu(QStringLiteral("◫ ") +
                                        I18n::t(QStringLiteral("MCP Servers")));
            for (const auto &s : servers) {
                auto *act = mcpSub->addAction(s.name, this, [this, s]() {
                    // attach an MCP prompt marker so the model can use it
                    Q_UNUSED(s);
                    m_composer->focusInput();
                });
                act->setCheckable(true);
                act->setChecked(s.enabled);
            }
            mcpSub->addSeparator();
            mcpSub->addAction(I18n::t(QStringLiteral("Configure servers…")),
                              this, [this]() { openMcp(); });
        }
        menu.addAction(QStringLiteral("💬 ") + I18n::t(QStringLiteral("System Message")),
                       this, [this]() {
                           bool ok = false;
                           const QString text = QInputDialog::getMultiLineText(
                               this, I18n::t(QStringLiteral("System Message")),
                               I18n::t(QStringLiteral("Message")), QString(), &ok);
                           if (ok && !text.trimmed().isEmpty()) {
                               auto *conv = m_store->active();
                               if (!conv) newChat();
                               conv = m_store->active();
                               ConvMsg sm{uid(), QStringLiteral("system"),
                                          QStringLiteral("system"), text.trimmed()};
                               m_store->appendMessage(conv, sm);
                               ChatMessage cm{ChatMessage::System, sm.id, text.trimmed(),
                                              QStringLiteral("system"), QJsonArray(), false};
                               m_messages->addMessage(cm);
                           }
                       });
        menu.exec(QCursor::pos());
    });
    connect(m_composer, &Composer::attachmentPreviewRequested, this, [this](int idx) {
        const auto atts = m_composer->attachments();
        if (idx < 0 || idx >= atts.size()) return;
        const auto att = atts.at(idx).toObject();
        auto *dlg = new QDialog(this);
        dlg->setObjectName(QStringLiteral("appDialog"));
        dlg->setWindowTitle(att.value(QStringLiteral("name")).toString());
        dlg->setModal(true);
        dlg->resize(520, 480);
        auto *lay = new QVBoxLayout(dlg);
        if (att.value(QStringLiteral("image")).toBool()) {
            QPixmap pm;
            pm.loadFromData(QByteArray::fromBase64(
                                att.value(QStringLiteral("data")).toString().toLatin1()),
                            "PNG");
            auto *img = new QLabel(dlg);
            img->setAlignment(Qt::AlignCenter);
            if (!pm.isNull()) {
                img->setPixmap(pm.scaled(480, 400, Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation));
            } else {
                img->setText(I18n::t(QStringLiteral("Image preview unavailable")));
            }
            lay->addWidget(img, 1);
        } else {
            auto *txt = new QTextEdit(dlg);
            txt->setReadOnly(true);
            txt->setPlainText(QString::fromUtf8(
                QByteArray::fromBase64(att.value(QStringLiteral("data")).toString().toLatin1())));
            lay->addWidget(txt, 1);
        }
        auto *closeBtn = new QPushButton(I18n::t(QStringLiteral("Close")), dlg);
        closeBtn->setProperty("outline", true);
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
        lay->addWidget(closeBtn, 0, Qt::AlignRight);
        dlg->exec();
        dlg->deleteLater();
    });
    setAcceptDrops(true);

    // message actions (v2 ChatMessageActionIcons)
    connect(m_messages, &MessageView::copyRequested, this, &MainWindow::onCopy);
    connect(m_messages, &MessageView::regenerateRequested, this, &MainWindow::onRegenerate);
    connect(m_messages, &MessageView::editRequested, this, &MainWindow::onEdit);
    connect(m_messages, &MessageView::deleteRequested, this, &MainWindow::onDelete);
    connect(m_messages, &MessageView::continueRequested, this, [this](const QString &id) {
        auto *conv = m_store->active();
        if (!conv) return;
        int idx = -1;
        for (int i = 0; i < conv->messages.size(); ++i)
            if (conv->messages.at(i).id == id) { idx = i; break; }
        if (idx < 0) return;
        // v2 "continue generation": truncate after this message and ask
        // the model to continue
        conv->messages = conv->messages.mid(0, idx + 1);
        m_store->save();
        renderConversation(conv);
        sendMessageWithHistory(QStringLiteral("Continue."));
    });

    // streaming
    connect(m_api, &Api::chunkReceived, this, [this](const QString &delta) {
        hideErrorBanner();
        m_completionChars += delta.size();
        updateGauge();
        m_messages->updateStreaming(delta);
        if (m_store->active() && !m_store->active()->messages.isEmpty()) {
            auto &last = m_store->active()->messages.last();
            last.content += delta;
        }
    });
    connect(m_api, &Api::reasoningReceived, this,
            [this](const QString &delta) { m_messages->updateReasoning(delta); });
    connect(m_api, &Api::contentDone, this, &MainWindow::onStreamFinished);
    connect(m_api, &Api::toolCallsReady, this, &MainWindow::onToolCalls);
    connect(m_api, &Api::requestFailed, this, [this](const QString &code, const QString &msg) {
        m_streaming = false;
        m_composer->setBusy(false);
        showErrorBanner(msg.isEmpty() ? QStringLiteral("LLMUI-") + code : msg);
    });

    // models
    connect(m_api, &Api::modelsLoaded, this, [this](const QJsonArray &models) {
        QStringList rawIds;
        for (const auto &m : models)
            rawIds << m.toObject().value(QStringLiteral("id")).toString();
        if (!rawIds.isEmpty()) {
            const QString preferred = Config::value(QStringLiteral("model")).toString();
            // v2 ModelBadge: append quantization + modality tags to the
            // short id (Q8_0/Q4_K_M, vision/reasoning)
            QStringList items;
            const QRegularExpression quant(R"((Q\d+_\d+_?(?:K_\d+|I_\d+|M)?|Q\d+_\d+|FP16|F16|BF16|IQ\d+_\w+))");
            for (const auto &id : rawIds) {
                QString shortId = id;
                const int slash = shortId.lastIndexOf('/');
                if (slash >= 0) shortId = shortId.mid(slash + 1);
                shortId.replace(QStringLiteral(".gguf"), QString());
                QString tags;
                const auto qm = quant.match(shortId);
                if (qm.hasMatch()) tags += QStringLiteral("  [") + qm.captured(1) + QStringLiteral("]");
                if (shortId.contains(QStringLiteral("VL")) || shortId.contains(QStringLiteral("Vision")))
                    tags += QStringLiteral(" · vision");
                if (shortId.contains(QStringLiteral("Thinking")) || shortId.contains(QStringLiteral("Reasoning")))
                    tags += QStringLiteral(" · reasoning");
                items << shortId + tags;
            }
            m_composer->setModels(items, rawIds,
                                  rawIds.contains(preferred) ? preferred : rawIds.first());
        }
    });
    m_api->fetchModels();

    // context gauge (UI-SPEC §5.7) — n_ctx from /props, char-based tokens
    m_nCtx = 4096;
    connect(m_api, &Api::propsLoaded, this, [this](int nCtx) {
        m_nCtx = nCtx;
        updateGauge();
    });
    m_api->fetchProps();

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

    // recompute the floating-form spacer once the window has settled
    QTimer::singleShot(400, this, [this]() { setEmpty(m_empty); });

    // restore last conversation if any
    if (!m_store->conversations().isEmpty()) {
        m_store->setActive(&m_store->conversations().first());
        selectConversation(m_store->active()->id);
    } else {
        setEmpty(true);
    }

    applyTheme(m_config.value(QStringLiteral("theme")).toString());
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    // keep the floating form at ~50dvh on the empty chat (v2)
    if (m_formSpacer && m_empty) {
        const int h = height();
        const int desiredBottom = h / 2 + 112;
        const int spacer = h - desiredBottom - 8;
        m_formSpacer->setFixedHeight(qMax(20, spacer));
    }
}

/** v2: the composer floats at ~50dvh on the empty chat and pins to the
 *  bottom once messages exist. */
void MainWindow::setEmpty(bool empty) {
    m_empty = empty;
    if (m_formSpacer) {
        m_formSpacer->setVisible(empty);
        if (empty) {
            // v2: form bottom sits at 50dvh - 7rem from the window bottom
            const int h = height();
            const int desiredBottom = h / 2 + 112; // bottom edge y of the form
            const int spacer = h - desiredBottom - 8; // 8px bottom margin
            m_formSpacer->setFixedHeight(qMax(20, spacer));
        }
    }
}

void MainWindow::applyTheme(const QString &paletteId) {
    qApp->setStyleSheet(Theme::stylesheet(paletteId));
    const auto t = Theme::tokens(paletteId);
    Icons::setColor(t.mutedForeground);
    m_sidebar->applyIcons();
    m_composer->applyIcons();
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
    m_composer->setPersonaName(I18n::t(QStringLiteral("Default")));
    refreshTitle();
    m_sidebar->refreshConversations();
    m_composer->focusInput();
    setEmpty(true);
}

void MainWindow::selectConversation(const QString &id) {
    for (auto &c : m_store->conversations()) {
        if (c.id == id) {
            m_store->setActive(&c);
            m_sidebar->setActiveConversation(id);
            renderConversation(&c);
            // persona trigger shows the active preset name (v2)
            const auto raw = Presets::load(m_config);
            QString name = I18n::t(QStringLiteral("Default"));
            if (!c.persona.isEmpty()) {
                name = QStringLiteral("Custom");
                for (const auto &p : Presets::list(raw))
                    if (p.content == c.persona) name = p.name;
            }
            m_composer->setPersonaName(name);
            refreshTitle();
            return;
        }
    }
}

void MainWindow::renderConversation(Conversation *conv) {
    m_messages->clear();
    const bool empty = conv->persona.isEmpty() && conv->messages.isEmpty();
    setEmpty(empty);
    if (empty) {
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

void MainWindow::updateGauge() {
    m_composer->setContextUsage(m_promptChars, m_completionChars, m_nCtx);
}

void MainWindow::onSend(const QString &text) {
    sendMessageWithHistory(text);
}

void MainWindow::abortForTest() {
    m_api->abort();
    m_composer->setBusy(false);
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
    setEmpty(false);

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

    // token accounting for the context gauge: prompt = full history
    m_promptChars = conv->persona.size();
    m_completionChars = 0;
    for (const auto &m : conv->messages)
        m_promptChars += m.content.size();
    updateGauge();

    QJsonArray payload = buildHistoryPayload(conv);
    m_api->startCompletion(payload, m_model, 0.7, toolSchemas());
}

void MainWindow::onStreamFinished() {
    m_streaming = false;
    m_composer->setBusy(false);
    if (m_store->active()) {
        // drop a placeholder assistant message left by a user stop (the
        // agentic continuation placeholder follows a tool message — kept)
        auto &msgs = m_store->active()->messages;
        const bool prevIsUser =
            msgs.size() >= 2 && msgs.at(msgs.size() - 2).role == QStringLiteral("user");
        if (!msgs.isEmpty() && msgs.last().role == QStringLiteral("assistant") &&
            msgs.last().content.trimmed().isEmpty() && (msgs.size() == 1 || prevIsUser))
            msgs.removeLast();
        m_store->save();
    }
    m_sidebar->refreshConversations();
    m_api->checkHealth();
}

/** Agentic loop (v2 tool calling): the model emitted tool_calls — execute
 *  them and continue the conversation (max 8 turns). */
void MainWindow::onToolCalls(const QJsonArray &toolCalls) {
    if (toolCalls.isEmpty()) return;
    if (m_toolTurn >= 8) {
        m_toolTurn = 0;
        return;
    }
    ++m_toolTurn;
    m_pendingTools = toolCalls;
    runAgenticTurn(toolCalls);
}

void MainWindow::runAgenticTurn(const QJsonArray &toolCalls) {
    auto *conv = m_store->active();
    if (!conv) return;

    // the sendMessageWithHistory placeholder is empty after a pure
    // tool-call turn — drop it so history stays clean
    if (!conv->messages.isEmpty()) {
        const auto &last = conv->messages.last();
        if (last.role == QStringLiteral("assistant") && last.content.trimmed().isEmpty())
            conv->messages.removeLast();
    }

    // assistant turn that requested the tools (persisted as type tool-call)
    QJsonArray callsPayload;
    for (const auto &c : toolCalls) {
        const auto o = c.toObject();
        callsPayload.append(QJsonObject{
            {QStringLiteral("id"), o.value(QStringLiteral("id")).toString()},
            {QStringLiteral("type"), QStringLiteral("function")},
            {QStringLiteral("function"),
             QJsonObject{
                 {QStringLiteral("name"), o.value(QStringLiteral("name")).toString()},
                 {QStringLiteral("arguments"), o.value(QStringLiteral("arguments")).toString()}}}});
    }
    ConvMsg toolMsg{uid(), QStringLiteral("assistant"), QStringLiteral("tool-call"),
                    QString::fromUtf8(QJsonDocument(callsPayload).toJson(QJsonDocument::Compact))};
    m_store->appendMessage(conv, toolMsg);

    // execute each tool, store the results, render them
    for (const auto &c : toolCalls) {
        const auto o = c.toObject();
        const QString name = o.value(QStringLiteral("name")).toString();
        const QString args = o.value(QStringLiteral("arguments")).toString();
        const QJsonObject resObj = executeTool(name, args);
        const QString result = resObj.value(QStringLiteral("result")).toString();
        ConvMsg res{uid(), QStringLiteral("tool"), QStringLiteral("tool-result"), result};
        m_store->appendMessage(conv, res);

        ChatMessage cm;
        cm.id = res.id;
        cm.type = QStringLiteral("tool-result");
        cm.role = ChatMessage::Assistant;
        cm.content = QStringLiteral("🔧 %1 → %2").arg(name, result.left(200));
        m_messages->addMessage(cm);
    }

    // continue: re-request with full history incl. tool results — with a
    // streaming placeholder so chunks land in the right message
    ConvMsg cont{uid(), QStringLiteral("assistant"), QStringLiteral("text"), QString()};
    m_store->appendMessage(conv, cont);
    ChatMessage ph{ChatMessage::Assistant, cont.id, QString(),
                   QStringLiteral("text"), QJsonArray(), true};
    m_messages->addMessage(ph);

    m_streaming = true;
    m_composer->setBusy(true);
    QJsonArray payload = buildHistoryPayload(conv);
    m_api->startCompletion(payload, m_model);
}

/** Execute a built-in tool app-side (no server round-trip). */
QJsonObject MainWindow::executeTool(const QString &name, const QString &arguments) {
    QJsonObject parsed = QJsonDocument::fromJson(arguments.toUtf8()).object();
    if (name == QStringLiteral("get_datetime")) {
        const QString fmt = parsed.value(QStringLiteral("format")).toString();
        return QJsonObject{{QStringLiteral("result"),
                            QDateTime::currentDateTime().toString(
                                fmt.isEmpty() ? QStringLiteral("yyyy-MM-dd HH:mm:ss") : fmt)}};
    }
    if (name == QStringLiteral("calculate")) {
        // safe arithmetic evaluator: digits/operators/parens only
        const QString expr = parsed.value(QStringLiteral("expression")).toString();
        QString e = expr;
        e.remove(QRegularExpression(QStringLiteral("[^0-9+\\-*/(). ]")));
        e.replace(QRegularExpression(QStringLiteral("(?<!\\d)-")), QStringLiteral("(0-1)*"));
        bool ok = false;
        const double r = safeEval(e, &ok);
        return QJsonObject{{QStringLiteral("result"), ok ? QString::number(r, 'g', 12)
                                                        : QStringLiteral("error: invalid expression")}};
    }
    if (name == QStringLiteral("fetch_url")) {
        const QString url = parsed.value(QStringLiteral("url")).toString();
        QNetworkRequest req{QUrl(url)};
        auto *nam = new QNetworkAccessManager(this);
        auto *reply = nam->get(req);
        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(15000, &loop, &QEventLoop::quit);
        loop.exec();
        const QByteArray data = reply->readAll();
        reply->deleteLater();
        nam->deleteLater();
        if (reply->error() != QNetworkReply::NoError)
            return QJsonObject{{QStringLiteral("result"),
                                QStringLiteral("error: ") + reply->errorString()}};
        QString text = QString::fromUtf8(data);
        text = text.left(4000);
        text.remove(QRegularExpression(QStringLiteral("<script[\\s\\S]*?</script>")));
        text.remove(QRegularExpression(QStringLiteral("<style[\\s\\S]*?</style>")));
        text.remove(QRegularExpression(QStringLiteral("<[^>]+>")));
        return QJsonObject{{QStringLiteral("result"), text.trimmed().left(3000)}};
    }
    return QJsonObject{{QStringLiteral("result"),
                        QStringLiteral("error: unknown tool %1").arg(name)}};
}

/** Build the OpenAI payload from stored history, reconstructing assistant
 *  tool_calls blocks and tool results (v2 agentic format). */
QJsonArray MainWindow::buildHistoryPayload(Conversation *conv) const {
    QJsonArray messages;
    if (!conv->persona.isEmpty())
        messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("system")},
                                     {QStringLiteral("content"), conv->persona}});
    for (const auto &m : conv->messages) {
        if (m.role == QStringLiteral("assistant") && m.content.isEmpty()) continue;
        if (m.role == QStringLiteral("system")) continue;
        if (m.type == QStringLiteral("tool-call")) {
            const auto calls = QJsonDocument::fromJson(m.content.toUtf8()).array();
            messages.append(QJsonObject{
                {QStringLiteral("role"), QStringLiteral("assistant")},
                {QStringLiteral("content"), QJsonValue(QJsonValue::Null)},
                {QStringLiteral("tool_calls"), calls}});
            continue;
        }
        if (m.role == QStringLiteral("tool")) {
            messages.append(QJsonObject{
                {QStringLiteral("role"), QStringLiteral("tool")},
                {QStringLiteral("tool_call_id"), m.id},
                {QStringLiteral("content"), m.content}});
            continue;
        }
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
    return messages;
}

void MainWindow::showErrorBanner(const QString &text) {
    m_errorBannerDetail->setText(text);
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
    // finish when the LAST assistant message has real content (the agentic
    // loop may produce several intermediate turns without content)
    *conn = connect(m_api, &Api::contentDone, this, [this, conn, shotPath]() {
        const auto &msgs = m_store->active()->messages;
        const bool hasAnswer = !msgs.isEmpty() &&
                               msgs.last().role == QStringLiteral("assistant") &&
                               !msgs.last().content.trimmed().isEmpty();
        if (!hasAnswer) return; // wait for the final turn
        QObject::disconnect(*conn);
        delete conn;
        {
            QFile dbg(QStringLiteral("/tmp/opencode/v4-html-dump.txt"));
            if (dbg.open(QIODevice::WriteOnly))
                dbg.write(m_messages->lastAssistantHtml().toUtf8());
        }
        const auto img = grab().toImage().scaledToWidth(900, Qt::SmoothTransformation);
        img.save(shotPath, "JPG", 70);
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
    input->setPlaceholderText(I18n::t(QStringLiteral("Search conversations...")));
    lay->addWidget(input);
    auto *results = new QListWidget(dlg);
    results->setObjectName(QStringLiteral("searchResults"));
    lay->addWidget(results, 1);
    auto *closeBtn = new QPushButton(I18n::t(QStringLiteral("Close")), dlg);
    closeBtn->setProperty("outline", true);
    connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    lay->addWidget(closeBtn, 0, Qt::AlignRight);

    auto populate = [results, this](const QString &q) {
        results->clear();
        const QString needle = q.trimmed();
        if (needle.isEmpty()) {
            auto *hint = new QListWidgetItem(I18n::t(QStringLiteral("Start typing to see results")));
            hint->setFlags(Qt::NoItemFlags);
            results->addItem(hint);
            return;
        }
        int convHits = 0, msgHits = 0;
        // conversation rows (v2: sidebar conversation rows, name match)
        for (const auto &c : m_store->conversations()) {
            if (c.name.contains(needle, Qt::CaseInsensitive)) {
                ++convHits;
                auto *item = new QListWidgetItem(QStringLiteral("💬 ") + c.name);
                item->setData(Qt::UserRole, c.id);
                results->addItem(item);
            }
        }
        // message hits with snippets
        for (const auto &c : m_store->conversations()) {
            for (const auto &m : c.messages) {
                if (m.content.contains(needle, Qt::CaseInsensitive)) {
                    ++msgHits;
                    QString snippet = m.content.mid(0, 90).replace('\n', ' ') +
                                      (m.content.size() > 90 ? QStringLiteral("…") : QString());
                    auto *item = new QListWidgetItem(
                        QStringLiteral("↳ %1: “%2”").arg(c.name, snippet));
                    item->setData(Qt::UserRole, c.id);
                    results->addItem(item);
                }
            }
        }
        if (convHits + msgHits == 0) {
            auto *none = new QListWidgetItem(I18n::t(QStringLiteral("No results found")));
            none->setFlags(Qt::NoItemFlags);
            results->addItem(none);
        }
    };
    connect(input, &QLineEdit::textChanged, dlg, [populate](const QString &q) {
        populate(q);
    });
    connect(results, &QListWidget::itemClicked, this, [this, dlg](QListWidgetItem *item) {
        const QString id = item->data(Qt::UserRole).toString();
        if (!id.isEmpty()) {
            selectConversation(id);
            dlg->close();
        }
    });
    populate(QString());
    dlg->exec();
    dlg->deleteLater();
}

void MainWindow::openMcp() {
    McpPage dlg(m_config, this);
    connect(&dlg, &McpPage::configChanged, this, [this](const QJsonObject &cfg) {
        m_config = cfg;
        Config::save(cfg);
    });
    dlg.exec();
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
    const auto presets = Presets::list(raw);
    auto *conv = m_store->active();

    auto activeName = [&]() -> QString {
        if (conv && !conv->persona.isEmpty()) {
            for (const auto &p : presets)
                if (p.content == conv->persona) return p.name;
            return QStringLiteral("Custom");
        }
        return QString();
    };

    // v2 menu structure: label, Default (check), favorites, separator,
    // "All presets…"
    auto *label = menu->addAction(I18n::t(QStringLiteral("Prompt presets")));
    label->setEnabled(false);
    auto *def = menu->addAction(I18n::t(QStringLiteral("Default")));
    def->setCheckable(true);
    def->setChecked(conv && conv->persona.isEmpty());
    connect(def, &QAction::triggered, this, [this, conv]() {
        if (!conv) return;
        conv->persona.clear();
        m_store->save();
        renderConversation(conv);
        m_composer->setPersonaName(I18n::t(QStringLiteral("Default")));
    });
    menu->addSeparator();

    const int favCount = qMin(presets.size(), 8);
    const QString active = activeName();
    for (int i = 0; i < favCount; ++i) {
        const auto &p = presets.at(i);
        auto *act = menu->addAction(p.name, this, [this, conv, p]() {
            if (!conv) return;
            conv->persona = p.content;
            m_store->save();
            ConvMsg pm{uid(), QStringLiteral("system"), QStringLiteral("persona"), p.content};
            m_store->appendMessage(conv, pm);
            ChatMessage cm{ChatMessage::System, pm.id, p.content, QStringLiteral("persona"),
                           QJsonArray(), false};
            m_messages->addMessage(cm);
            m_composer->setPersonaName(p.name);
        });
        act->setCheckable(true);
        act->setChecked(active == p.name);
    }
    menu->addSeparator();
    menu->addAction(I18n::t(QStringLiteral("All presets…")), this, [this, presets]() {
        // picker dialog: searchable list + Default + create-with-wizard
        auto *dlg = new QDialog(this);
        dlg->setObjectName(QStringLiteral("appDialog"));
        dlg->setWindowTitle(I18n::t(QStringLiteral("Prompt presets")));
        dlg->setModal(true);
        dlg->resize(420, 480);
        auto *lay = new QVBoxLayout(dlg);
        auto *search = new QLineEdit(dlg);
        search->setPlaceholderText(I18n::t(QStringLiteral("Search presets…")));
        lay->addWidget(search);
        auto *list = new QListWidget(dlg);
        list->setObjectName(QStringLiteral("presetList"));
        auto populate = [list, presets](const QString &needle) {
            list->clear();
            for (const auto &p : presets) {
                if (!needle.isEmpty() &&
                    !p.name.contains(needle, Qt::CaseInsensitive) &&
                    !p.description.contains(needle, Qt::CaseInsensitive))
                    continue;
                auto *item = new QListWidgetItem(p.name);
                item->setData(Qt::UserRole, p.content);
                item->setToolTip(p.description);
                list->addItem(item);
            }
        };
        populate(QString());
        connect(search, &QLineEdit::textChanged, list, [populate](const QString &q) {
            populate(q);
        });
        lay->addWidget(list, 1);
        auto *foot = new QHBoxLayout;
        auto *wizardBtn = new QPushButton(QStringLiteral("✦ ") + I18n::t(QStringLiteral("Create with wizard…")), dlg);
        wizardBtn->setProperty("ghost", true);
        connect(wizardBtn, &QPushButton::clicked, dlg, [this, dlg]() {
            WizardDialog wiz(m_api, m_model, dlg);
            connect(&wiz, &WizardDialog::presetCreated, this,
                    [this](const QString &name, const QString &desc, const QString &content) {
                        auto raw = Presets::load(m_config);
                        m_config.insert(QStringLiteral("systemPromptPresets"),
                                        Presets::add(raw, name, desc, content));
                        Config::save(m_config);
                    });
            wiz.exec();
        });
        foot->addWidget(wizardBtn);
        foot->addStretch(1);
        auto *closeBtn = new QPushButton(I18n::t(QStringLiteral("Close")), dlg);
        closeBtn->setProperty("outline", true);
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
        foot->addWidget(closeBtn);
        lay->addLayout(foot);
        connect(list, &QListWidget::itemClicked, this, [this, dlg](QListWidgetItem *item) {
            auto *conv = m_store->active();
            if (!conv) return;
            conv->persona = item->data(Qt::UserRole).toString();
            m_store->save();
            ConvMsg pm{uid(), QStringLiteral("system"), QStringLiteral("persona"), conv->persona};
            m_store->appendMessage(conv, pm);
            ChatMessage cm{ChatMessage::System, pm.id, conv->persona,
                           QStringLiteral("persona"), QJsonArray(), false};
            m_messages->addMessage(cm);
            m_composer->setPersonaName(item->text());
            dlg->close();
        });
        dlg->exec();
        dlg->deleteLater();
    });
    menu->exec(QCursor::pos());
}
