#include "Sidebar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

Sidebar::Sidebar(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("sidebar"));
    setFixedWidth(56); // w-12 collapsed strip

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 10, 6, 10);
    layout->setSpacing(4);

    // Logo button (spec: h-9 w-9 rounded-full)
    auto *logo = makeStripButton(QStringLiteral("◆"), QStringLiteral("Llama UI Native"));
    logo->setObjectName(QStringLiteral("logoButton"));
    layout->addWidget(logo, 0, Qt::AlignHCenter);

    layout->addStretch(1);

    auto *newChat = makeStripButton(QStringLiteral("✎"), QStringLiteral("New chat"));
    connect(newChat, &QPushButton::clicked, this, &Sidebar::newChatClicked);
    layout->addWidget(newChat, 0, Qt::AlignHCenter);

    auto *search = makeStripButton(QStringLiteral("⌕"), QStringLiteral("Search"));
    search->setEnabled(false); // search page comes later
    layout->addWidget(search, 0, Qt::AlignHCenter);

    auto *mcp = makeStripButton(QStringLiteral("◫"), QStringLiteral("MCP Servers"));
    mcp->setEnabled(false);
    layout->addWidget(mcp, 0, Qt::AlignHCenter);

    auto *settings = makeStripButton(QStringLiteral("⚙"), QStringLiteral("Settings"));
    connect(settings, &QPushButton::clicked, this, &Sidebar::settingsClicked);
    layout->addWidget(settings, 0, Qt::AlignHCenter);

    layout->addStretch(1);
}

QPushButton *Sidebar::makeStripButton(const QString &icon, const QString &tooltip) {
    auto *btn = new QPushButton(icon, this);
    btn->setProperty("strip", true);
    btn->setFixedSize(32, 32);
    btn->setToolTip(tooltip);
    return btn;
}
