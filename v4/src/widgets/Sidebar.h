/*
 * Sidebar.h — v2 SidebarNavigation: floating rounded panel, w-12 collapsed
 * (transparent) / w-72 expanded (bg-muted/60 + border + shadow). Ghost
 * action rows (New chat / Search / MCP / Settings) with kbd hints, inline
 * search mode, conversation list, pinned headers, active highlight,
 * context menu (pin/edit/export/delete).
 */
#pragma once
#include <QWidget>

class ConversationStore;
class QPushButton;
class QVBoxLayout;
class QLineEdit;
class QLabel;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(ConversationStore *store, QWidget *parent = nullptr);
    void expandForTest() { setExpanded(true); }

    void refreshConversations();
    void setActiveConversation(const QString &id);
    void setServerStatus(bool online, const QString &address);
    void toggleExpanded() { setExpanded(!m_expanded); }
    void activateSearchMode();

signals:
    void newChatClicked();
    void settingsClicked();
    void searchClicked();
    void mcpClicked();
    void conversationSelected(const QString &id);
    void conversationRenameRequested(const QString &id);
    void conversationDeleteRequested(const QString &id);
    void toggleExpandedRequested();

private:
    void setExpanded(bool expanded);
    void rebuildActions();
    void rebuildList();
    void applyPanelStyle();

    ConversationStore *m_store;
    QVBoxLayout *m_outer;
    QPushButton *m_logo;
    QPushButton *m_closeBtn;
    QVBoxLayout *m_actionsLayout;
    QVBoxLayout *m_listLayout;
    QLineEdit *m_searchInput = nullptr;
    QLabel *m_statusLabel = nullptr;
    QString m_activeId;
    bool m_expanded = false;
    bool m_searchMode = false;
};
