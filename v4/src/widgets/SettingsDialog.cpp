#include "SettingsDialog.h"
#include "../i18n.h"
#include "../Theme.h"
#include <QComboBox>
#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SettingsDialog::SettingsDialog(QJsonObject cfg, QWidget *parent)
    : QDialog(parent), m_cfg(std::move(cfg)) {
    setObjectName(QStringLiteral("appDialog"));
    setWindowTitle(QStringLiteral("Settings"));
    setModal(true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto *title = new QLabel(I18n::t(QStringLiteral("Settings")));
    title->setObjectName(QStringLiteral("dialogTitle"));
    layout->addWidget(title);

    auto *form = new QFormLayout;
    form->setSpacing(10);

    m_lang = new QComboBox(this);
    for (const auto &code : I18n::languages())
        m_lang->addItem(code, code);
    m_lang->setCurrentIndex(qMax(0, m_lang->findData(m_cfg.value(QStringLiteral("language")).toString())));
    form->addRow(QStringLiteral("Language"), m_lang);

    m_theme = new QComboBox(this);
    m_theme->addItem(QStringLiteral("System"), QStringLiteral("system"));
    m_theme->addItem(QStringLiteral("Dark"), QStringLiteral("dark"));
    m_theme->addItem(QStringLiteral("Light"), QStringLiteral("light"));
    for (const auto &p : Theme::palettes())
        m_theme->addItem(p.id, p.id);
    m_theme->setCurrentIndex(qMax(0, m_theme->findData(m_cfg.value(QStringLiteral("theme")).toString())));
    form->addRow(QStringLiteral("Theme"), m_theme);

    m_endpoint = new QLineEdit(m_cfg.value(QStringLiteral("serverEndpoint")).toString(), this);
    form->addRow(QStringLiteral("Server Endpoint"), m_endpoint);

    m_temperature = new QLineEdit(m_cfg.value(QStringLiteral("temperature")).toString(), this);
    m_temperature->setPlaceholderText(QStringLiteral("Default: 0.8"));
    form->addRow(QStringLiteral("Temperature"), m_temperature);

    m_topK = new QLineEdit(m_cfg.value(QStringLiteral("top_k")).toString(), this);
    m_topK->setPlaceholderText(QStringLiteral("Default: 40"));
    form->addRow(QStringLiteral("Top K"), m_topK);

    m_topP = new QLineEdit(m_cfg.value(QStringLiteral("top_p")).toString(), this);
    m_topP->setPlaceholderText(QStringLiteral("Default: 0.95"));
    form->addRow(QStringLiteral("Top P"), m_topP);

    m_systemMessage = new QLineEdit(m_cfg.value(QStringLiteral("systemMessage")).toString(), this);
    form->addRow(QStringLiteral("Default system prompt"), m_systemMessage);

    layout->addLayout(form);

    auto *buttons = new QHBoxLayout;
    auto *cancel = new QPushButton(I18n::t(QStringLiteral("Cancel")), this);
    cancel->setProperty("outline", true);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancel);
    buttons->addStretch(1);
    auto *save = new QPushButton(I18n::t(QStringLiteral("Save settings")), this);
    connect(save, &QPushButton::clicked, this, &SettingsDialog::apply);
    buttons->addWidget(save);
    layout->addLayout(buttons);
}

void SettingsDialog::apply() {
    m_cfg.insert(QStringLiteral("language"), m_lang->currentData().toString());
    m_cfg.insert(QStringLiteral("theme"), m_theme->currentData().toString());
    m_cfg.insert(QStringLiteral("serverEndpoint"), m_endpoint->text().trimmed());
    m_cfg.insert(QStringLiteral("temperature"), m_temperature->text().trimmed());
    m_cfg.insert(QStringLiteral("top_k"), m_topK->text().trimmed());
    m_cfg.insert(QStringLiteral("top_p"), m_topP->text().trimmed());
    m_cfg.insert(QStringLiteral("systemMessage"), m_systemMessage->text());
    emit configChanged(m_cfg);
    accept();
}
