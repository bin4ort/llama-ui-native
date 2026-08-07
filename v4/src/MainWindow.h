/*
 * MainWindow.h — v4 main window: spec sidebar strip + chat screen
 * (greeting/messages/composer), spec theme applied from config,
 * conversation persistence.
 */
#pragma once
#include <QComboBox>
#include <QJsonObject>
#include <QMainWindow>

class Api;
class ConversationStore;
class MessageView;
class Composer;
class Sidebar;
class SettingsDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void expandSidebarForTest();
    void overrideThemeForTest(const QString &id);
    void openSettingsForTest();

public slots:
    /** E2E self-test: new chat -> send message -> wait for reply -> save
     *  screenshot (vision verification loop). */
    void runSelfTest(const QString &message, const QString &shotPath);

signals:
    void selfTestDone();

private slots:
    void onSend(const QString &text);
    void openSettings();
    void openPersonaMenu();
    void openSearch();
    void openMcp();

private:
    void applyTheme(const QString &paletteId);
    void sendMessage(const QString &text);
    void newChat();
    void selectConversation(const QString &id);
    void refreshTitle();

    Api *m_api;
    ConversationStore *m_store;
    Sidebar *m_sidebar;
    MessageView *m_messages;
    Composer *m_composer;
    QComboBox *m_modelCombo = nullptr;
    QJsonObject m_config;
    QString m_model;
    bool m_streaming = false;
};
