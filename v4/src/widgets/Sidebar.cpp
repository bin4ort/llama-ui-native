#include "Sidebar.h"
#include "../ConversationStore.h"
#include "../Icons.h"
#include "../i18n.h"
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPushButton>
#include <QSize>
#include <QScrollArea>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>

namespace {
const char *kVersion = "0.6.0-dev";
}

Sidebar::Sidebar(ConversationStore *store, QWidget *parent)
    : QWidget(parent), m_store(store) {
    setObjectName(QStringLiteral("sidebar"));
    setAttribute(Qt::WA_StyledBackground);
    setFixedWidth(48); // v2: collapsed strip w-12

    m_outer = new QVBoxLayout(this);
    m_outer->setContentsMargins(8, 8, 0, 6); // v2: aside p-2 area — logo at x16
    m_outer->setSpacing(0);

    // logo row: expand/collapse toggle + close button when expanded
    // (hosted in a fixed-height widget — a bare layout-in-layout gets a
    // broken sizeHint in Qt6 and collapses to 8px, shifting everything)
    auto *topHost = new QWidget(this);
    topHost->setFixedHeight(40);
    auto *topRow = new QHBoxLayout(topHost);
    topRow->setContentsMargins(8, 8, 4, 0); // v2: logo at (16,16), close at (248,16)
    topRow->setSpacing(0);
    m_logo = new QPushButton(topHost);
    m_logo->setProperty("strip", true);
    m_logo->setProperty("logoBtn", true);
    m_logo->setFixedSize(36, 36);
    m_logo->setIconSize(QSize(16, 16)); // v2: size-4 logo
    m_logo->setToolTip(I18n::t(QStringLiteral("Expand navigation")));
    connect(m_logo, &QPushButton::clicked, this, [this]() { setExpanded(!m_expanded); });
    topRow->addWidget(m_logo);
    topRow->addStretch(1);
    m_closeBtn = new QPushButton(topHost);
    m_closeBtn->setProperty("strip", true);
    m_closeBtn->setProperty("logoBtn", true);
    m_closeBtn->setFixedSize(36, 36);
    m_closeBtn->setIconSize(QSize(16, 16)); // v2: size-4
    m_closeBtn->setToolTip(I18n::t(QStringLiteral("Collapse navigation")));
    m_closeBtn->setVisible(false);
    connect(m_closeBtn, &QPushButton::clicked, this, [this]() { setExpanded(false); });
    topRow->addWidget(m_closeBtn);
    m_outer->addWidget(topHost);

    // action rows (v2 SIDEBAR_ACTIONS_ITEMS: New chat / Search / MCP / Settings)
    auto *actionsHost = new QWidget(this);
    m_actionsLayout = new QVBoxLayout(actionsHost);
    m_actionsLayout->setContentsMargins(8, 4, 0, 0); // adjusted per state in setExpanded // v2: buttons at y52 — Qt6 paints QSS button content +8 low
    m_actionsLayout->setSpacing(4); // v2: gap-1 between action rows (40px pitch)
    m_outer->addWidget(actionsHost);
    rebuildActions();

    // conversation list
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *listHost = new QWidget;
    m_listLayout = new QVBoxLayout(listHost);
    m_listLayout->setContentsMargins(0, 30, 0, 0);
    m_listLayout->setSpacing(2);
    scroll->setWidget(listHost);
    m_outer->addWidget(scroll, 1);
    rebuildList();

    // server status footer (kept subtle, spec §2.5)
    m_statusLabel = new QLabel(this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setVisible(false);
    m_outer->addWidget(m_statusLabel);

    applyPanelStyle();
    applyIcons();
}

/** v2: collapsed strip is a transparent floating panel; expanded gets the
 *  muted/60 background + border + shadow. */
void Sidebar::applyPanelStyle() {
    setProperty("expanded", m_expanded);
    style()->unpolish(this);
    style()->polish(this);
}

void Sidebar::setExpanded(bool expanded) {
    m_expanded = expanded;
    setFixedWidth(expanded ? 288 : 48); // v2: w-72 / w-12
    m_closeBtn->setVisible(expanded);
    m_logo->setToolTip(expanded ? I18n::t(QStringLiteral("Collapse navigation"))
                                : I18n::t(QStringLiteral("Expand navigation")));
    applyPanelStyle();
    if (m_actionsLayout)
        m_actionsLayout->setContentsMargins(8, expanded ? 4 : 12, 0, 0);
    if (expanded) {
        refreshConversations();
        rebuildActions();
    }
}

void Sidebar::activateSearchMode() {
    if (!m_expanded) setExpanded(true);
    m_searchMode = true;
    rebuildActions();
    if (m_searchInput) {
        m_searchInput->setFocus();
        m_searchInput->show();
    }
    rebuildList();
}

void Sidebar::rebuildActions() {
    while (m_actionsLayout->count()) {
        auto *item = m_actionsLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    // v2: SearchInput appears above the actions in search mode
    if (m_searchMode && m_expanded) {
        m_searchInput = new QLineEdit(this);
        m_searchInput->setObjectName(QStringLiteral("sidebarSearch"));
        m_searchInput->setPlaceholderText(I18n::t(QStringLiteral("Search conversations...")));
        m_actionsLayout->addWidget(m_searchInput);
    } else {
        m_searchInput = nullptr;
    }

    struct Action {
        QString icon, label, kbd, slot;
    };
    const QList<Action> actions{
        {QStringLiteral("square-pen"), I18n::t(QStringLiteral("New chat")),
         I18n::t(QStringLiteral("Ctrl+Shift+O")), QStringLiteral("new")},
        {QStringLiteral("search"), I18n::t(QStringLiteral("Search")),
         I18n::t(QStringLiteral("Ctrl+K")), QStringLiteral("search")},
        {QStringLiteral("mcp"), I18n::t(QStringLiteral("MCP Servers")),
         QString(), QStringLiteral("mcp")},
        {QStringLiteral("settings"), I18n::t(QStringLiteral("Settings")),
         QString(), QStringLiteral("settings")},
    };
    for (const auto &a : actions) {
        auto *btn = new QPushButton(this);
        btn->setProperty("sideAction", true);
        btn->setProperty("iconName", a.icon);
        if (qEnvironmentVariableIsSet("LLMUI_NOICON")) { btn->setIconSize(QSize(16,16)); }
        else btn->setIconSize(QSize(16, 16)); // v2: size-4 icons
        btn->setMinimumHeight(36); // v2: h-9 action rows
        btn->setMaximumHeight(36);
        btn->setFixedWidth(m_expanded ? 272 : 36); // v2: w-full rows / w-9 strip
        btn->setContentsMargins(m_expanded ? QMargins(11, 0, 16, 0) : QMargins(0, 0, 0, 0));
        if (m_expanded) {
            // v2: the kbd hint is hidden until hover
            btn->setText(a.label);
        }
        btn->setToolTip(a.label + (a.kbd.isEmpty() ? QString() : QStringLiteral("  (%1)").arg(a.kbd)));
        connect(btn, &QPushButton::clicked, this, [this, a]() {
            if (a.slot == QStringLiteral("new")) emit newChatClicked();
            else if (a.slot == QStringLiteral("search")) {
                if (m_expanded) {
                    m_searchMode = !m_searchMode;
                    if (!m_searchMode) m_searchInput = nullptr;
                    rebuildActions();
                    rebuildList();
                    if (m_searchMode && m_searchInput) m_searchInput->setFocus();
                } else {
                    emit searchClicked();
                }
            } else if (a.slot == QStringLiteral("mcp")) emit mcpClicked();
            else emit settingsClicked();
        });
        m_actionsLayout->addWidget(btn, 0, m_expanded ? Qt::AlignLeft : Qt::AlignHCenter);
    }
    if (m_searchMode && m_expanded && m_searchInput)
        connect(m_searchInput, &QLineEdit::textChanged, this, [this]() { rebuildList(); });
}

void Sidebar::refreshConversations() {
    if (!m_expanded) return;
    rebuildList();
}

void Sidebar::rebuildList() {
    if (!m_listLayout) return;
    while (m_listLayout->count()) {
        auto *item = m_listLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    const QString needle = m_searchInput ? m_searchInput->text().trimmed() : QString();
    const auto &convs = m_store->conversations();
    QList<Conversation> filtered;
    for (const auto &c : convs) {
        if (!needle.isEmpty() &&
            !c.name.contains(needle, Qt::CaseInsensitive))
            continue;
        filtered.append(c);
    }
    if (filtered.isEmpty()) {
        auto *empty = new QLabel(
            needle.isEmpty() ? I18n::t(QStringLiteral("No conversations yet"))
                             : I18n::t(QStringLiteral("No results found")));
        empty->setObjectName(QStringLiteral("convEmpty"));
        empty->setWordWrap(true);
        m_listLayout->addWidget(empty);
        m_listLayout->addStretch(1);
        return;
    }
    auto sorted = filtered;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const Conversation &a, const Conversation &b) {
                         return a.pinned && !b.pinned;
                     });
    bool inPinned = !sorted.isEmpty() && sorted.first().pinned;
    if (inPinned) {
        auto *l = new QLabel(I18n::t(QStringLiteral("Pinned")));
        l->setObjectName(QStringLiteral("convHeader"));
        m_listLayout->addWidget(l);
    }
    for (const auto &c : sorted) {
        if (inPinned && !c.pinned) {
            auto *l = new QLabel(I18n::t(QStringLiteral("Recent conversations")));
            l->setObjectName(QStringLiteral("convHeader"));
            m_listLayout->addWidget(l);
            inPinned = false;
        }
        auto *row = new QPushButton(c.name);
        row->setProperty("convRow", true);
        if (c.id == m_activeId) row->setProperty("active", true);
        row->setToolTip(c.name);
        row->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(row, &QPushButton::clicked, this, [this, id = c.id]() {
            emit conversationSelected(id);
        });
        connect(row, &QWidget::customContextMenuRequested, this, [this, row, c]() {
            QMenu menu(row);
            menu.addAction(c.pinned ? I18n::t(QStringLiteral("Unpin"))
                                    : I18n::t(QStringLiteral("Pin")),
                           this, [this, id = c.id, pinned = c.pinned]() {
                               for (auto &cc : m_store->conversations())
                                   if (cc.id == id) cc.pinned = !pinned;
                               m_store->save();
                               refreshConversations();
                           });
            menu.addAction(I18n::t(QStringLiteral("Edit")),
                           this, [this, id = c.id]() { emit conversationRenameRequested(id); });
            auto *expAct = menu.addAction(I18n::t(QStringLiteral("Export")),
                           this, [this, c]() {
                               const QString path = QFileDialog::getSaveFileName(
                                   this, I18n::t(QStringLiteral("Export conversation")),
                                   c.name + QStringLiteral(".json"));
                               if (path.isEmpty()) return;
                               QJsonArray msgs;
                               for (const auto &m : c.messages)
                                   msgs.append(QJsonObject{{QStringLiteral("role"), m.role},
                                                           {QStringLiteral("type"), m.type},
                                                           {QStringLiteral("content"), m.content}});
                               QFile f(path);
                               if (f.open(QIODevice::WriteOnly))
                                   f.write(QJsonDocument(QJsonObject{
                                       {QStringLiteral("name"), c.name},
                                       {QStringLiteral("messages"), msgs}})
                                               .toJson(QJsonDocument::Indented));
                           });
            expAct->setIcon(Icons::get(QStringLiteral("download"), Icons::color()));
            menu.addSeparator();
            menu.addAction(I18n::t(QStringLiteral("Delete")),
                           this, [this, id = c.id]() { emit conversationDeleteRequested(id); });
            menu.exec(QCursor::pos());
        });
        m_listLayout->addWidget(row);
    }
    m_listLayout->addStretch(1);
}

void Sidebar::setActiveConversation(const QString &id) {
    m_activeId = id;
    refreshConversations();
}

void Sidebar::applyIcons() {
    const QColor c = Icons::color();
    m_logo->setIcon(Icons::logo(c));
    m_closeBtn->setIcon(Icons::get(QStringLiteral("panel-left-close"), c));
    // refresh the action-row icons (rebuilt with the stored icon names)
    for (int i = 0; i < m_actionsLayout->count(); ++i) {
        auto *w = m_actionsLayout->itemAt(i)->widget();
        auto *btn = qobject_cast<QPushButton *>(w);
        if (!btn) continue;
        const QString icon = btn->property("iconName").toString();
        if (icon == QStringLiteral("mcp"))
            btn->setIcon(Icons::mcpLogo(c));
        else
            btn->setIcon(Icons::get(icon, c));
    }
}

void Sidebar::setServerStatus(bool online, const QString &address) {
    if (m_statusLabel) {
        m_statusLabel->setText(online
                                   ? I18n::t(QStringLiteral("Server online")) +
                                         QStringLiteral(" · ") + address
                                   : I18n::t(QStringLiteral("Server offline")));
        m_statusLabel->setVisible(true);
    }
}
