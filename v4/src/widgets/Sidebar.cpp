#include "Sidebar.h"
#include "../ConversationStore.h"
#include "../i18n.h"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

Sidebar::Sidebar(ConversationStore *store, QWidget *parent)
    : QWidget(parent), m_store(store) {
    setObjectName(QStringLiteral("sidebar"));
    setFixedWidth(56);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 10, 6, 10);
    layout->setSpacing(4);

    auto makeStrip = [](const QString &icon, const QString &tip) {
        auto *b = new QPushButton(icon);
        b->setProperty("strip", true);
        b->setFixedSize(36, 36);
        b->setFont(QFont(QStringLiteral("DejaVu Sans"), 15));
        b->setToolTip(tip);
        return b;
    };

    // Logo: expand/collapse toggle (spec: click expands the strip)
    m_logo = makeStrip(QStringLiteral("◆"), QStringLiteral("Expand navigation"));
    connect(m_logo, &QPushButton::clicked, this, [this]() { setExpanded(!m_expanded); });
    layout->addWidget(m_logo, 0, Qt::AlignHCenter);

    layout->addStretch(1);

    auto *newChat = makeStrip(QStringLiteral("✚"), QStringLiteral("New chat"));
    connect(newChat, &QPushButton::clicked, this, &Sidebar::newChatClicked);
    layout->addWidget(newChat, 0, Qt::AlignHCenter);

    auto *search = makeStrip(QStringLiteral("⌕"), QStringLiteral("Search"));
    connect(search, &QPushButton::clicked, this, &Sidebar::searchClicked);
    layout->addWidget(search, 0, Qt::AlignHCenter);

    auto *mcp = makeStrip(QStringLiteral("◫"), QStringLiteral("MCP Servers"));
    connect(mcp, &QPushButton::clicked, this, &Sidebar::mcpClicked);
    layout->addWidget(mcp, 0, Qt::AlignHCenter);

    auto *settings = makeStrip(QStringLiteral("⚙"), QStringLiteral("Settings"));
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
    m_expandedPanel->setVisible(false);
    layout->addWidget(m_expandedPanel, 1);

    setExpanded(false);
}

void Sidebar::setExpanded(bool expanded) {
    m_expanded = expanded;
    setFixedWidth(expanded ? 288 : 56);
    m_expandedPanel->setVisible(expanded);
    m_logo->setToolTip(expanded ? QStringLiteral("Collapse navigation")
                                : QStringLiteral("Expand navigation"));
    if (expanded) refreshConversations();
}

void Sidebar::refreshConversations() {
    if (!m_expanded) return;
    while (m_expandedLayout->count()) {
        auto *item = m_expandedLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    auto *newChatBtn = new QPushButton(I18n::t(QStringLiteral("New Chat")));
    newChatBtn->setProperty("outline", true);
    newChatBtn->setProperty("full", true);
    connect(newChatBtn, &QPushButton::clicked, this, &Sidebar::newChatClicked);
    m_expandedLayout->addWidget(newChatBtn);

    for (const auto &c : m_store->conversations()) {
        auto *row = new QPushButton(c.name);
        row->setProperty("convRow", true);
        row->setProperty("outline", true);
        row->setToolTip(c.name);
        connect(row, &QPushButton::clicked, this, [this, id = c.id]() {
            emit conversationSelected(id);
        });
        m_expandedLayout->addWidget(row);
    }
    m_expandedLayout->addStretch(1);
}
