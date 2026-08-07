/*
 * Sidebar.h — spec sidebar (UI-SPEC §3.17): collapsed icon strip (w-12)
 * that expands (w-72) into a conversation list panel.
 */
#pragma once
#include <QWidget>

class ConversationStore;
class QPushButton;
class QVBoxLayout;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(ConversationStore *store, QWidget *parent = nullptr);
    void expandForTest() { setExpanded(true); }

    void refreshConversations();

signals:
    void newChatClicked();
    void settingsClicked();
    void searchClicked();
    void mcpClicked();
    void conversationSelected(const QString &id);

private:
    void setExpanded(bool expanded);

    ConversationStore *m_store;
    QPushButton *m_logo;
    QVBoxLayout *m_stripLayout;
    QVBoxLayout *m_expandedLayout;
    QWidget *m_expandedPanel;
    bool m_expanded = false;
};
