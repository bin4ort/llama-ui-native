/*
 * MainWindow.h — v4 main window: spec sidebar strip + chat screen
 * (greeting/messages/composer), spec theme applied from config,
 * conversation persistence.
 */
#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>

class Api;
class ConversationStore;
class Conversation;
class MessageView;
class Composer;
class Sidebar;
class SettingsDialog;
class QLabel;
class QDragEnterEvent;
class QDropEvent;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void expandSidebarForTest();
    void overrideThemeForTest(const QString &id);
    void openSettingsForTest(int section = -1);
    void openMcpForTest() { openMcp(); }
    void openSearchForTest() { openSearch(); }
    void attachForTest(const QStringList &paths) { attachFiles(paths); }
    void abortForTest();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    /** E2E self-test: new chat -> send message -> wait for reply -> save
     *  screenshot (vision verification loop). */
    void runSelfTest(const QString &message, const QString &shotPath);

signals:
    void selfTestDone();

private slots:
    void onSend(const QString &text);
    void openSettings();
    void openSettings(int section);
    void openPersonaMenu();
    void openSearch();
    void openMcp();

private:
    void applyTheme(const QString &paletteId);
    void sendMessage(const QString &text);
    void sendMessageWithHistory(const QString &text);
    void newChat();
    void selectConversation(const QString &id);
    void renderConversation(Conversation *conv);
    void setEmpty(bool empty);
    void refreshTitle();
    void showErrorBanner(const QString &text);
    void hideErrorBanner();
    void setServerStatus(bool online);
    void onStreamFinished();
    void onCopy(const QString &id, const QString &content);
    void onRegenerate(const QString &id);
    void onEdit(const QString &id);
    void onDelete(const QString &id);
    void onRenameConversation(const QString &id);
    void onDeleteConversation(const QString &id);
    void attachFiles(const QStringList &paths);
    void onToolCalls(const QJsonArray &toolCalls);
    void runAgenticTurn(const QJsonArray &toolCalls);
    QJsonObject executeTool(const QString &name, const QString &arguments);
    QJsonArray buildHistoryPayload(Conversation *conv) const;

    Api *m_api;
    ConversationStore *m_store;
    Sidebar *m_sidebar;
    MessageView *m_messages;
    Composer *m_composer;
    QLabel *m_errorBanner = nullptr;
    QLabel *m_errorBannerDetail = nullptr;
    QWidget *m_formSpacer = nullptr;
    bool m_empty = true;
    QJsonObject m_config;
    QString m_model;
    bool m_streaming = false;
    int m_toolTurn = 0;
    QJsonArray m_pendingTools;
    QJsonObject m_propsDefaults;
    qint64 m_promptChars = 0;
    qint64 m_completionChars = 0;
    int m_nCtx = 4096;

    void updateGauge();
};
