#include "Sidebar.h"
#include "../ConversationStore.h"
#include "../i18n.h"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QVBoxLayout>
#include <algorithm>

namespace {
const char *kVersion = "0.6.0-dev";
}

Sidebar::Sidebar(ConversationStore *store, QWidget *parent)
    : QWidget(parent), m_store(store) {
    setObjectName(QStringLiteral("sidebar"));
    setFixedWidth(56);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 10, 6, 10);
    layout->setSpacing(4);

    auto makeStrip = [this](const QString &icon, const QString &tip,
                            const QString &shortcut = QString()) {
        auto *b = new QPushButton(icon);
        b->setProperty("strip", true);
        b->setFixedSize(36, 36);
        b->setFont(QFont(QStringLiteral("DejaVu Sans"), 15));
        b->setToolTip(shortcut.isEmpty() ? tip : tip + QStringLiteral("  (%1)").arg(shortcut));
        return b;
    };

    // Logo: expand/collapse toggle (spec: click expands the strip)
    m_logo = makeStrip(QStringLiteral("◆"), QStringLiteral("Expand navigation"),
                       I18n::t(QStringLiteral("Ctrl+B")));
    connect(m_logo, &QPushButton::clicked, this, [this]() { setExpanded(!m_expanded); });
    layout->addWidget(m_logo, 0, Qt::AlignHCenter);

    layout->addStretch(1);

    auto *newChat = makeStrip(QStringLiteral("✚"),
                              I18n::t(QStringLiteral("New chat")),
                              I18n::t(QStringLiteral("Ctrl+Shift+O")));
    connect(newChat, &QPushButton::clicked, this, &Sidebar::newChatClicked);
    layout->addWidget(newChat, 0, Qt::AlignHCenter);

    auto *search = makeStrip(QStringLiteral("⌕"), I18n::t(QStringLiteral("Search")),
                             I18n::t(QStringLiteral("Ctrl+K")));
    connect(search, &QPushButton::clicked, this, &Sidebar::searchClicked);
    layout->addWidget(search, 0, Qt::AlignHCenter);

    auto *mcp = makeStrip(QStringLiteral("◫"), I18n::t(QStringLiteral("MCP Servers")));
    connect(mcp, &QPushButton::clicked, this, &Sidebar::mcpClicked);
    layout->addWidget(mcp, 0, Qt::AlignHCenter);

    auto *settings = makeStrip(QStringLiteral("⚙"), I18n::t(QStringLiteral("Settings")));
    connect(settings, &QPushButton::clicked, this, &Sidebar::settingsClicked);
    layout->addWidget(settings, 0, Qt::AlignHCenter);

    layout->addStretch(1);

    // Expanded panel (conversation list)
    m_expandedPanel = new QWidget(this);
    m_expandedPanel->setObjectName(QStringLiteral("sidebarExpanded"));
    auto *el = new QVBoxLayout(m_expandedPanel);
    el->setContentsMargins(8, 8, 8, 8);
    el->setSpacing(4);
    m_expandedLayout = new QVBoxLayout;
    m_expandedLayout->setContentsMargins(0, 0, 0, 0);
    m_expandedLayout->setSpacing(2);
    auto *scroll = new QScrollArea(m_expandedPanel);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *listHost = new QWidget;
    listHost->setLayout(m_expandedLayout);
    scroll->setWidget(listHost);
    el->addWidget(scroll, 1);

    // Server status footer (UI-SPEC §2.5)
    auto *footer = new QWidget(m_expandedPanel);
    footer->setObjectName(QStringLiteral("sidebarFooter"));
    auto *fl = new QHBoxLayout(footer);
    fl->setContentsMargins(4, 6, 4, 2);
    fl->setSpacing(6);
    m_statusDot = new QLabel(QStringLiteral("●"), footer);
    m_statusDot->setObjectName(QStringLiteral("statusDot"));
    m_statusDot->setProperty("state", QStringLiteral("off"));
    fl->addWidget(m_statusDot);
    m_statusLabel = new QLabel(I18n::t(QStringLiteral("Server offline")), footer);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    fl->addWidget(m_statusLabel, 1);
    auto *ver = new QLabel(QStringLiteral("v%1").arg(kVersion), footer);
    ver->setObjectName(QStringLiteral("versionLabel"));
    fl->addWidget(ver);
    el->addWidget(footer);

    m_expandedPanel->setVisible(false);
    layout->addWidget(m_expandedPanel, 1);

    setExpanded(false);
}

void Sidebar::setExpanded(bool expanded) {
    m_expanded = expanded;
    setFixedWidth(expanded ? 288 : 56);
    m_expandedPanel->setVisible(expanded);
    m_logo->setToolTip(expanded ? I18n::t(QStringLiteral("Collapse navigation"))
                                : I18n::t(QStringLiteral("Expand navigation")));
    if (expanded) refreshConversations();
}

void Sidebar::setActiveConversation(const QString &id) {
    m_activeId = id;
    refreshConversations();
}

void Sidebar::setServerStatus(bool online, const QString &address) {
    m_statusDot->setProperty("state", online ? QStringLiteral("online") : QStringLiteral("off"));
    m_statusDot->style()->unpolish(m_statusDot);
    m_statusDot->style()->polish(m_statusDot);
    m_statusLabel->setText(online
                               ? I18n::t(QStringLiteral("Server online")) +
                                     QStringLiteral(" · ") + address
                               : I18n::t(QStringLiteral("Server offline")));
}

void Sidebar::refreshConversations() {
    if (!m_expanded) return;
    while (m_expandedLayout->count()) {
        auto *item = m_expandedLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto *newChatBtn = new QPushButton(I18n::t(QStringLiteral("New Chat")));
    newChatBtn->setProperty("primary", true);
    newChatBtn->setProperty("full", true);
    connect(newChatBtn, &QPushButton::clicked, this, &Sidebar::newChatClicked);
    m_expandedLayout->addWidget(newChatBtn);

    const auto &convs = m_store->conversations();
    if (convs.isEmpty()) {
        auto *empty = new QLabel(I18n::t(QStringLiteral("No conversations yet")));
        empty->setObjectName(QStringLiteral("convEmpty"));
        empty->setWordWrap(true);
        m_expandedLayout->addWidget(empty);
        m_expandedLayout->addStretch(1);
        return;
    }

    // pinned first, then recents (UI-SPEC §2.1)
    auto sorted = convs;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const Conversation &a, const Conversation &b) {
                         return a.pinned && !b.pinned;
                     });
    const bool anyPinned = !sorted.isEmpty() && sorted.first().pinned;
    bool inPinned = anyPinned;
    if (anyPinned) {
        auto *l = new QLabel(I18n::t(QStringLiteral("Pinned")));
        l->setObjectName(QStringLiteral("convHeader"));
        m_expandedLayout->addWidget(l);
    }
    for (const auto &c : sorted) {
        if (inPinned && !c.pinned) {
            auto *l = new QLabel(I18n::t(QStringLiteral("Recent conversations")));
            l->setObjectName(QStringLiteral("convHeader"));
            m_expandedLayout->addWidget(l);
            inPinned = false;
        }
        auto *row = new QPushButton(c.pinned ? QStringLiteral("📌 ") + c.name : c.name);
        row->setProperty("convRow", true);
        if (c.id == m_activeId) row->setProperty("active", true);
        row->setToolTip(c.name);
        row->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(row, &QPushButton::clicked, this, [this, id = c.id]() {
            emit conversationSelected(id);
        });
        connect(row, &QWidget::customContextMenuRequested, this, [this, row, id = c.id]() {
            QMenu menu(row);
            menu.addAction(I18n::t(QStringLiteral("Rename")),
                           this, [this, id]() { emit conversationRenameRequested(id); });
            menu.addAction(I18n::t(QStringLiteral("Delete")),
                           this, [this, id]() { emit conversationDeleteRequested(id); });
            menu.exec(QCursor::pos());
        });
        m_expandedLayout->addWidget(row);
    }
    m_expandedLayout->addStretch(1);
}
