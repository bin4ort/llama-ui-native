/*
 * Sidebar.h — spec sidebar (UI-SPEC §3.17): collapsed icon strip (w-12)
 * that expands (w-72) into a conversation list panel with pinned header,
 * active highlight, per-row context menu (rename/delete) and a
 * server-status footer (UI-SPEC §2.5).
 */
#pragma once
#include <QWidget>

class ConversationStore;
class QPushButton;
class QVBoxLayout;
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

    ConversationStore *m_store;
    QPushButton *m_logo;
    QVBoxLayout *m_expandedLayout;
    QWidget *m_expandedPanel;
    QLabel *m_statusDot;
    QLabel *m_statusLabel;
    QString m_activeId;
    bool m_expanded = false;
};
