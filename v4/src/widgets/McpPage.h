/*
 * McpPage.h — MCP Servers page (UI-SPEC §10.1): title + description,
 * "Add server" button, server cards with connection status, edit/delete,
 * empty state. Native dialog shell (no router in the native build).
 */
#pragma once
#include <QDialog>
#include <QJsonObject>

class QVBoxLayout;
class QLabel;

class McpPage : public QDialog {
    Q_OBJECT
public:
    explicit McpPage(const QJsonObject &config, QWidget *parent = nullptr);

signals:
    void configChanged(const QJsonObject &cfg);

private:
    void rebuild();
    void addServer();
    void editServer(int index);
    void deleteServer(int index);
    void testServer(int index, QLabel *badge);

    QJsonObject m_cfg;
    QVBoxLayout *m_list;
};
