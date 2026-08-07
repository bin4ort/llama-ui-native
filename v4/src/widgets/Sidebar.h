/*
 * Sidebar.h — spec sidebar (UI-SPEC §3.17): collapsed icon strip (w-12,
 * rounded-2xl) with logo + New chat / Search / MCP / Settings buttons.
 */
#pragma once
#include <QWidget>
class QPushButton;

class Sidebar : public QWidget {
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

signals:
    void newChatClicked();
    void settingsClicked();

private:
    QPushButton *makeStripButton(const QString &icon, const QString &tooltip);
};
