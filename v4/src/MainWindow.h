/*
 * MainWindow.h — v4 main window: spec sidebar strip + chat screen
 * (greeting/messages/composer), spec theme applied from config.
 */
#pragma once
#include <QMainWindow>
#include <QJsonObject>

class Api;
class MessageView;
class Composer;
class Sidebar;
class SettingsDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSend(const QString &text);
    void openSettings();

private:
    void applyTheme(const QString &paletteId);
    void sendMessage(const QString &text, const QString &systemMessage);

    Api *m_api;
    Sidebar *m_sidebar;
    MessageView *m_messages;
    Composer *m_composer;
    QJsonObject m_config;
    QString m_model;
    bool m_streaming = false;
};
