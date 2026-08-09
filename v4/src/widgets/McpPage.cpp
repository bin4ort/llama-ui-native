#include "McpPage.h"
#include <cstdio>
#include "../Icons.h"
#include "../i18n.h"
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStyle>
#include <QScrollArea>
#include <QTimer>
#include <QSize>
#include <QVBoxLayout>

QJsonArray serversOf(const QJsonObject &cfg) {
    const QJsonValue v = cfg.value(QStringLiteral("mcpServers"));
    if (v.isArray()) return v.toArray();
    if (v.isString())
        return QJsonDocument::fromJson(v.toString().toUtf8()).array();
    return QJsonArray();
}

namespace McpServers {
QList<Server> list(const QJsonObject &cfg) {
    QList<Server> out;
    for (const auto &v : serversOf(cfg)) {
        const auto o = v.toObject();
        Server s;
        s.name = o.value(QStringLiteral("name")).toString();
        s.url = o.value(QStringLiteral("url")).toString();
        s.enabled = o.value(QStringLiteral("enabled")).toBool(true);
        out.append(s);
    }
    return out;
}
} // namespace McpServers

McpPage::McpPage(const QJsonObject &config, QWidget *parent)
    : QDialog(parent), m_cfg(config) {
    setObjectName(QStringLiteral("appDialog"));
    setWindowTitle(I18n::t(QStringLiteral("MCP Servers")));
    setModal(true);
    resize(1400, 900);

    auto *outer = new QHBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // v2: collapsed sidebar strip at x0-56, content pane after it
    auto *navPad = new QWidget(this);
    navPad->setFixedWidth(56); // v2: w-12 strip + ml-2
    navPad->setAutoFillBackground(false);
    auto *stripL = new QVBoxLayout(navPad);
    stripL->setContentsMargins(10, 28, 10, 0);
    stripL->setSpacing(0);
    auto stripBtn = [this, navPad](const QString &icon, const QString &tip, bool sel = false) {
        auto *b = new QPushButton(navPad);
        b->setProperty("strip", true);
        b->setProperty("mcpActive", sel);
        b->setFixedSize(28, 28);
        b->setIconSize(QSize(14, 14));
        b->setIcon(Icons::get(icon, Icons::color()));
        b->setToolTip(tip);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };
    auto *stripLogo = new QPushButton(navPad);
    stripLogo->setProperty("strip", true);
    stripLogo->setProperty("logoBtn", true);
    stripLogo->setFixedSize(28, 28);
    stripLogo->setIconSize(QSize(12, 12));
    stripLogo->setIcon(Icons::logo(Icons::color()));
    stripLogo->setToolTip(I18n::t(QStringLiteral("Back to Chat")));
    connect(stripLogo, &QPushButton::clicked, this, &QDialog::close);
    stripL->addWidget(stripLogo, 0, Qt::AlignHCenter);
    stripL->addSpacing(16);
    auto *b1 = stripBtn(QStringLiteral("message-square"), I18n::t(QStringLiteral("New Chat")));
    connect(b1, &QPushButton::clicked, this, &QDialog::close);
    stripL->addWidget(b1, 0, Qt::AlignHCenter);
    stripL->addSpacing(16);
    auto *b2 = stripBtn(QStringLiteral("search"), I18n::t(QStringLiteral("Search")));
    connect(b2, &QPushButton::clicked, this, &QDialog::close);
    stripL->addWidget(b2, 0, Qt::AlignHCenter);
    stripL->addSpacing(16);
    auto *b3 = stripBtn(QStringLiteral("boxes"), I18n::t(QStringLiteral("MCP servers")), true);
    stripL->addWidget(b3, 0, Qt::AlignHCenter);
    stripL->addSpacing(16);
    auto *b4 = stripBtn(QStringLiteral("settings"), I18n::t(QStringLiteral("Settings")));
    stripL->addWidget(b4, 0, Qt::AlignHCenter);
    stripL->addStretch(1);
    outer->addWidget(navPad);

    auto *content = new QWidget(this);
    auto *col = new QVBoxLayout(content);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(0);

    // v2 full-page layout: sticky header (mt-4, h64) + centered content
    auto *header = new QWidget(content);
    header->setObjectName(QStringLiteral("mcpHeader"));
    header->setFixedHeight(64);
    auto *hl = new QHBoxLayout(header);
    hl->setContentsMargins(2, 16, 32, 12); // v2: title at x58
    auto *titleIcon = new QLabel(content);
    titleIcon->setPixmap(Icons::mcpLogo(Icons::color()).pixmap(20, 20));
    hl->addWidget(titleIcon);
    auto *title = new QLabel(I18n::t(QStringLiteral("MCP Servers")), content);
    title->setObjectName(QStringLiteral("mcpTitle"));
    hl->addWidget(title);
    hl->addStretch(1);
    auto *closeBtn = new QPushButton(content);
    closeBtn->setProperty("ghost", true);
    closeBtn->setIcon(Icons::get(QStringLiteral("x"), Icons::color()));
    closeBtn->setIconSize(QSize(18, 18));
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    hl->addWidget(closeBtn);
    col->addWidget(header);

    auto *scroll = new QScrollArea(content);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    m_list = new QVBoxLayout(host);
    m_list->setContentsMargins(32, 8, 32, 24);
    m_list->setSpacing(10);
    m_list->setProperty("emptyMode", false);
    scroll->setWidget(host);
    col->addWidget(scroll, 1);
    outer->addWidget(content, 1);
    rebuild();
}

void McpPage::rebuild() {
    while (m_list->count()) {
        auto *item = m_list->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    const auto servers = serversOf(m_cfg);
    if (servers.isEmpty()) {
        // v2 empty state: 16px plus icon + 14px title/desc + primary button
        m_list->setSpacing(6);
        m_list->addStretch(1);
        auto *icon = new QLabel(this);
        icon->setObjectName(QStringLiteral("mcpEmptyIcon"));
        icon->setPixmap(Icons::get(QStringLiteral("plus"), Icons::color()).pixmap(16, 16));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(24, 24);
        m_list->addWidget(icon, 0, Qt::AlignHCenter);
        m_list->addSpacing(4);
        auto *t1 = new QLabel(I18n::t(QStringLiteral("Add your first MCP server")), this);
        t1->setObjectName(QStringLiteral("mcpEmptyTitle"));
        t1->setAlignment(Qt::AlignCenter);
        m_list->addWidget(t1);
        auto *t2 = new QLabel(I18n::t(QStringLiteral("Connect a remote MCP server by URL.")), this);
        t2->setObjectName(QStringLiteral("mcpEmptyDesc"));
        t2->setAlignment(Qt::AlignCenter);
        m_list->addWidget(t2);
        m_list->addSpacing(8);
        auto *btn = new QPushButton(QStringLiteral("+ Add new server"), this);
        btn->setProperty("primary", true);
        btn->setMinimumWidth(160);
        connect(btn, &QPushButton::clicked, this, &McpPage::addServer);
        m_list->addWidget(btn, 0, Qt::AlignHCenter);
        m_list->addStretch(1);
        return;
    }
    fprintf(stderr, "[MCP] rebuild: %d servers\n", servers.size());
    for (int i = 0; i < servers.size(); ++i) {
        const auto s = servers.at(i).toObject();
        const QString name = s.value(QStringLiteral("name")).toString();
        const QString url = s.value(QStringLiteral("url")).toString();
        const bool enabled = s.value(QStringLiteral("enabled")).toBool(true);

        auto *card = new QWidget(this);
        card->setObjectName(QStringLiteral("mcpCard"));
        card->setAttribute(Qt::WA_StyledBackground);
        card->setMinimumHeight(64);
        auto *vl = new QVBoxLayout(card);
        vl->setContentsMargins(12, 10, 12, 10);
        vl->setSpacing(4);
        // header: McpLogo + name + transport badge + state + actions
        // (v2 McpServerCardHeader)
        auto *row1 = new QHBoxLayout;
        auto *logoLbl = new QLabel(card);
        logoLbl->setPixmap(Icons::mcpLogo(Icons::color()).pixmap(20, 20));
        row1->addWidget(logoLbl);
        auto *nameLbl = new QLabel(name, card);
        nameLbl->setObjectName(QStringLiteral("mcpName"));
        row1->addWidget(nameLbl);
        auto *transport = new QLabel(QStringLiteral("SSE"), card);
        transport->setObjectName(QStringLiteral("mcpTransport"));
        row1->addWidget(transport);
        auto *state = new QLabel(
            enabled ? I18n::t(QStringLiteral("Enabled")) : I18n::t(QStringLiteral("Disabled")),
            card);
        state->setObjectName(QStringLiteral("mcpState"));
        state->setProperty("on", enabled);
        row1->addWidget(state);
        row1->addStretch(1);
        auto *editBtn = new QPushButton(card);
        editBtn->setProperty("ghost", true);
        editBtn->setIcon(Icons::get(QStringLiteral("pencil"), Icons::color()));
        editBtn->setIconSize(QSize(14, 14));
        editBtn->setToolTip(I18n::t(QStringLiteral("Edit")));
        connect(editBtn, &QPushButton::clicked, this, [this, i]() { editServer(i); });
        row1->addWidget(editBtn);
        auto *delBtn = new QPushButton(card);
        delBtn->setProperty("ghost", true);
        delBtn->setIcon(Icons::get(QStringLiteral("trash-2"), Icons::color()));
        delBtn->setIconSize(QSize(14, 14));
        delBtn->setToolTip(I18n::t(QStringLiteral("Delete")));
        connect(delBtn, &QPushButton::clicked, this, [this, i]() { deleteServer(i); });
        row1->addWidget(delBtn);
        vl->addLayout(row1);
        // capability badges (v2 McpCapabilitiesBadges)
        auto *badges = new QHBoxLayout;
        badges->setContentsMargins(0, 0, 0, 0);
        badges->setSpacing(6);
        const QList<QPair<QString, QString>> caps{
            {QStringLiteral("Tools"), QStringLiteral("green")},
            {QStringLiteral("Resources"), QStringLiteral("blue")},
            {QStringLiteral("Prompts"), QStringLiteral("purple")},
        };
        for (const auto &[label, color] : caps) {
            auto *b = new QLabel(label, card);
            b->setObjectName(QStringLiteral("mcpCap"));
            b->setProperty("cap", color);
            badges->addWidget(b);
        }
        badges->addStretch(1);
        vl->addLayout(badges);
        auto *urlLbl = new QLabel(url, card);
        urlLbl->setObjectName(QStringLiteral("mcpUrl"));
        vl->addWidget(urlLbl);
        // collapsible tools list (v2 McpServerCardToolsList)
        auto *toolsBtn = new QPushButton(
            QStringLiteral("▾ ") + I18n::t(QStringLiteral("Show details")), card);
        toolsBtn->setProperty("ghost", true);
        toolsBtn->setObjectName(QStringLiteral("mcpToolsBtn"));
        toolsBtn->setCheckable(true);
        auto *toolsList = new QLabel(
            QStringLiteral("• ") + I18n::t(QStringLiteral("No tools reported")), card);
        toolsList->setObjectName(QStringLiteral("mcpToolsList"));
        toolsList->setVisible(false);
        connect(toolsBtn, &QPushButton::toggled, this, [toolsBtn, toolsList](bool checked) {
            toolsBtn->setText(QString(checked ? u"▴ " : u"▾ ") +
                              I18n::t(QStringLiteral("Show details")));
            toolsList->setVisible(checked);
        });
        vl->addWidget(toolsBtn);
        vl->addWidget(toolsList);
        m_list->addWidget(card);
    }
    m_list->addStretch(1);
}

void McpPage::addServer() {
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("appDialog"));
    dlg->setWindowTitle(I18n::t(QStringLiteral("Add MCP server")));
    dlg->setModal(true);
    auto *lay = new QVBoxLayout(dlg);
    auto *name = new QLineEdit(dlg);
    name->setPlaceholderText(I18n::t(QStringLiteral("Server name")));
    lay->addWidget(name);
    auto *url = new QLineEdit(dlg);
    url->setPlaceholderText(I18n::t(QStringLiteral("http://localhost:3001/mcp")));
    lay->addWidget(url);
    auto *row = new QHBoxLayout;
    auto *cancel = new QPushButton(I18n::t(QStringLiteral("Cancel")), dlg);
    cancel->setProperty("outline", true);
    connect(cancel, &QPushButton::clicked, dlg, &QDialog::reject);
    row->addWidget(cancel);
    row->addStretch(1);
    auto *save = new QPushButton(I18n::t(QStringLiteral("Save settings")), dlg);
    connect(save, &QPushButton::clicked, dlg, [dlg, name, url]() {
        if (!name->text().trimmed().isEmpty() && !url->text().trimmed().isEmpty())
            dlg->accept();
    });
    row->addWidget(save);
    lay->addLayout(row);
    if (dlg->exec() != QDialog::Accepted) return;

    auto servers = serversOf(m_cfg);
    servers.append(QJsonObject{
        {QStringLiteral("name"), name->text().trimmed()},
        {QStringLiteral("url"), url->text().trimmed()},
        {QStringLiteral("enabled"), true},
    });
    m_cfg.insert(QStringLiteral("mcpServers"),
                 QString::fromUtf8(QJsonDocument(servers).toJson(QJsonDocument::Indented)));
    rebuild();
    emit configChanged(m_cfg);
}

void McpPage::editServer(int index) {
    auto servers = serversOf(m_cfg);
    if (index < 0 || index >= servers.size()) return;
    const auto s = servers.at(index).toObject();
    auto *dlg = new QDialog(this);
    dlg->setObjectName(QStringLiteral("appDialog"));
    dlg->setWindowTitle(I18n::t(QStringLiteral("Edit MCP server")));
    dlg->setModal(true);
    auto *lay = new QVBoxLayout(dlg);
    auto *name = new QLineEdit(s.value(QStringLiteral("name")).toString(), dlg);
    lay->addWidget(name);
    auto *url = new QLineEdit(s.value(QStringLiteral("url")).toString(), dlg);
    lay->addWidget(url);
    auto *row = new QHBoxLayout;
    auto *cancel = new QPushButton(I18n::t(QStringLiteral("Cancel")), dlg);
    cancel->setProperty("outline", true);
    connect(cancel, &QPushButton::clicked, dlg, &QDialog::reject);
    row->addWidget(cancel);
    row->addStretch(1);
    auto *save = new QPushButton(I18n::t(QStringLiteral("Save settings")), dlg);
    connect(save, &QPushButton::clicked, dlg, [dlg]() { dlg->accept(); });
    row->addWidget(save);
    lay->addLayout(row);
    if (dlg->exec() != QDialog::Accepted) return;
    QJsonObject updated = s;
    updated.insert(QStringLiteral("name"), name->text().trimmed());
    updated.insert(QStringLiteral("url"), url->text().trimmed());
    servers.replace(index, updated);
    m_cfg.insert(QStringLiteral("mcpServers"),
                 QString::fromUtf8(QJsonDocument(servers).toJson(QJsonDocument::Indented)));
    rebuild();
    emit configChanged(m_cfg);
}

void McpPage::deleteServer(int index) {
    auto servers = serversOf(m_cfg);
    if (index < 0 || index >= servers.size()) return;
    const auto ret = QMessageBox::question(
        this, I18n::t(QStringLiteral("Delete MCP server")),
        I18n::t(QStringLiteral("Are you sure?")));
    if (ret != QMessageBox::Yes) return;
    servers.removeAt(index);
    m_cfg.insert(QStringLiteral("mcpServers"),
                 QString::fromUtf8(QJsonDocument(servers).toJson(QJsonDocument::Indented)));
    rebuild();
    emit configChanged(m_cfg);
}

void McpPage::testServer(int index, QLabel *badge) {
    const auto servers = serversOf(m_cfg);
    if (index < 0 || index >= servers.size()) return;
    const QString url = servers.at(index).toObject().value(QStringLiteral("url")).toString();
    badge->setText(I18n::t(QStringLiteral("Testing…")));
    auto *nam = new QNetworkAccessManager(this);
    QNetworkRequest req{QUrl(url)};
    auto *reply = nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, nam, badge]() {
        reply->deleteLater();
        nam->deleteLater();
        const bool ok = reply->error() == QNetworkReply::NoError;
        badge->setProperty("on", ok);
        badge->style()->unpolish(badge);
        badge->style()->polish(badge);
        badge->setText(ok ? I18n::t(QStringLiteral("Reachable"))
                          : I18n::t(QStringLiteral("Unreachable")));
    });
}
