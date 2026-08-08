#include "McpPage.h"
#include <cstdio>
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

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    // v2 full-page layout: header top-left, centered content
    auto *header = new QWidget(this);
    header->setObjectName(QStringLiteral("mcpHeader"));
    auto *hl = new QHBoxLayout(header);
    hl->setContentsMargins(24, 16, 24, 12);
    auto *title = new QLabel(QStringLiteral("◫ ") + I18n::t(QStringLiteral("MCP Servers")), this);
    title->setObjectName(QStringLiteral("mcpTitle"));
    hl->addWidget(title);
    hl->addStretch(1);
    auto *closeBtn = new QPushButton(QStringLiteral("✕"), this);
    closeBtn->setProperty("ghost", true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    hl->addWidget(closeBtn);
    outer->addWidget(header);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    m_list = new QVBoxLayout(host);
    m_list->setContentsMargins(24, 8, 24, 24);
    m_list->setSpacing(10);
    scroll->setWidget(host);
    outer->addWidget(scroll, 1);
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
        // v2 empty state: Plus icon circle + title + desc + primary button
        m_list->addStretch(1);
        auto *icon = new QLabel(QStringLiteral("＋"), this);
        icon->setObjectName(QStringLiteral("mcpEmptyIcon"));
        icon->setAlignment(Qt::AlignCenter);
        icon->setFixedSize(48, 48);
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
        auto *btn = new QPushButton(I18n::t(QStringLiteral("Add New Server")), this);
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
        auto *row1 = new QHBoxLayout;
        auto *nameLbl = new QLabel(name, card);
        nameLbl->setObjectName(QStringLiteral("mcpName"));
        row1->addWidget(nameLbl);
        auto *state = new QLabel(
            enabled ? I18n::t(QStringLiteral("Enabled")) : I18n::t(QStringLiteral("Disabled")),
            card);
        state->setObjectName(QStringLiteral("mcpState"));
        state->setProperty("on", enabled);
        row1->addWidget(state);
        row1->addStretch(1);
        auto *testBtn = new QPushButton(I18n::t(QStringLiteral("Test connection")), card);
        testBtn->setProperty("ghost", true);
        connect(testBtn, &QPushButton::clicked, this, [this, i, state]() {
            testServer(i, state);
        });
        row1->addWidget(testBtn);
        auto *editBtn = new QPushButton(I18n::t(QStringLiteral("Edit")), card);
        editBtn->setProperty("ghost", true);
        connect(editBtn, &QPushButton::clicked, this, [this, i]() { editServer(i); });
        row1->addWidget(editBtn);
        auto *delBtn = new QPushButton(I18n::t(QStringLiteral("Delete")), card);
        delBtn->setProperty("ghost", true);
        connect(delBtn, &QPushButton::clicked, this, [this, i]() { deleteServer(i); });
        row1->addWidget(delBtn);
        vl->addLayout(row1);
        auto *urlLbl = new QLabel(url, card);
        urlLbl->setObjectName(QStringLiteral("mcpUrl"));
        vl->addWidget(urlLbl);
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
