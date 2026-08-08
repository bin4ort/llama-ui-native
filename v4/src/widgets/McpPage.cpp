#include "McpPage.h"
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

namespace {
/** mcpServers is stored as a JSON *string* (same key as the web app). */
QJsonArray serversOf(const QJsonObject &cfg) {
    const QJsonValue v = cfg.value(QStringLiteral("mcpServers"));
    if (v.isArray()) return v.toArray();
    if (v.isString())
        return QJsonDocument::fromJson(v.toString().toUtf8()).array();
    return QJsonArray();
}
}

McpPage::McpPage(const QJsonObject &config, QWidget *parent)
    : QDialog(parent), m_cfg(config) {
    setObjectName(QStringLiteral("appDialog"));
    setWindowTitle(I18n::t(QStringLiteral("MCP Servers")));
    setModal(true);
    resize(560, 420);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(20, 20, 20, 20);
    outer->setSpacing(12);

    auto *title = new QLabel(I18n::t(QStringLiteral("MCP Servers")), this);
    title->setObjectName(QStringLiteral("dialogTitle"));
    outer->addWidget(title);
    auto *desc = new QLabel(
        I18n::t(QStringLiteral(
            "Connect Model Context Protocol servers to give the assistant access to external tools and resources.")),
        this);
    desc->setObjectName(QStringLiteral("dialogDesc"));
    desc->setWordWrap(true);
    outer->addWidget(desc);

    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    m_list = new QVBoxLayout(host);
    m_list->setContentsMargins(0, 0, 0, 0);
    m_list->setSpacing(10);
    scroll->setWidget(host);
    outer->addWidget(scroll, 1);
    rebuild();

    auto *bottom = new QHBoxLayout;
    bottom->addStretch(1);
    auto *closeBtn = new QPushButton(I18n::t(QStringLiteral("Close")), this);
    closeBtn->setProperty("outline", true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::close);
    bottom->addWidget(closeBtn);
    outer->addLayout(bottom);
}

void McpPage::rebuild() {
    while (m_list->count()) {
        auto *item = m_list->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    const auto servers = serversOf(m_cfg);
    if (servers.isEmpty()) {
        // v2 empty state: centered card with primary add button
        auto *empty = new QWidget(this);
        empty->setObjectName(QStringLiteral("mcpEmpty"));
        auto *vl = new QVBoxLayout(empty);
        vl->setContentsMargins(16, 24, 16, 24);
        vl->setSpacing(8);
        auto *t1 = new QLabel(I18n::t(QStringLiteral("Add your first MCP server")), empty);
        t1->setObjectName(QStringLiteral("mcpEmptyTitle"));
        t1->setAlignment(Qt::AlignCenter);
        vl->addWidget(t1);
        auto *t2 = new QLabel(I18n::t(QStringLiteral("Connect a remote MCP server by URL.")), empty);
        t2->setObjectName(QStringLiteral("mcpEmptyDesc"));
        t2->setAlignment(Qt::AlignCenter);
        vl->addWidget(t2);
        auto *btn = new QPushButton(I18n::t(QStringLiteral("Add New Server")), empty);
        btn->setProperty("primary", true);
        connect(btn, &QPushButton::clicked, this, &McpPage::addServer);
        vl->addWidget(btn, 0, Qt::AlignCenter);
        m_list->addWidget(empty);
        return;
    }
    for (int i = 0; i < servers.size(); ++i) {
        const auto s = servers.at(i).toObject();
        const QString name = s.value(QStringLiteral("name")).toString();
        const QString url = s.value(QStringLiteral("url")).toString();
        const bool enabled = s.value(QStringLiteral("enabled")).toBool(true);

        auto *card = new QWidget(this);
        card->setObjectName(QStringLiteral("mcpCard"));
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
