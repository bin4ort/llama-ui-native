#include "WizardDialog.h"
#include "../Api.h"
#include "../i18n.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

// Wizard meta-prompt (same contract as the web app's presets-wizard).
static const QString META =
    QStringLiteral(
        "You are a senior system prompt engineer. Turn the user's description into a DETAILED, "
        "professional system prompt that works well with local LLMs. It must be 150-400 words with "
        "clear sections (ROLE, HOW TO INTERACT / METHOD, RESPONSE FORMAT, RULES), concrete "
        "techniques of the field, exact session behavior and answer formats. Write in second "
        "person imperative. No robotic filler, no cop-outs like \"consult a specialist\" (the "
        "persona must ACT AS the expert requested; the only exception is genuine safety-critical "
        "situations). Respond with JSON only: {\"description\": \"one line\", \"content\": \"the "
        "system prompt\"}");

WizardDialog::WizardDialog(Api *api, const QString &model, QWidget *parent)
    : QDialog(parent), m_api(api), m_model(model) {
    setObjectName(QStringLiteral("appDialog"));
    setWindowTitle(QStringLiteral("Create a prompt preset"));
    setModal(true);
    resize(560, 620);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("Create a prompt preset"));
    title->setObjectName(QStringLiteral("dialogTitle"));
    layout->addWidget(title);
    auto *desc = new QLabel(
        QStringLiteral("Describe the personality or expert role — the model drafts the system prompt for you to review."));
    desc->setObjectName(QStringLiteral("dialogDesc"));
    desc->setWordWrap(true);
    layout->addWidget(desc);

    m_request = new QTextEdit;
    m_request->setFixedHeight(70);
    m_request->setPlaceholderText(QStringLiteral("A psychologist who helps me untangle a decision…"));
    layout->addWidget(m_request);

    auto *genRow = new QHBoxLayout;
    auto *generate = new QPushButton(QStringLiteral("Generate"));
    connect(generate, &QPushButton::clicked, this, &WizardDialog::generate);
    genRow->addWidget(generate);
    genRow->addStretch(1);
    layout->addLayout(genRow);

    auto *draftTitle = new QLabel(QStringLiteral("System prompt draft"));
    draftTitle->setObjectName(QStringLiteral("dialogDesc"));
    layout->addWidget(draftTitle);

    m_draft = new QTextEdit;
    m_draft->setPlaceholderText(QStringLiteral("The generated system prompt appears here for review…"));
    m_draft->setFixedHeight(200);
    layout->addWidget(m_draft);

    auto *form = new QFormLayout;
    m_name = new QLineEdit;
    form->addRow(QStringLiteral("Preset name"), m_name);
    m_description = new QLineEdit;
    form->addRow(QStringLiteral("Description"), m_description);
    layout->addLayout(form);

    auto *buttons = new QHBoxLayout;
    auto *cancel = new QPushButton(I18n::t(QStringLiteral("Cancel")));
    cancel->setProperty("outline", true);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancel);
    buttons->addStretch(1);
    auto *save = new QPushButton(QStringLiteral("Save preset"));
    connect(save, &QPushButton::clicked, this, &WizardDialog::save);
    buttons->addWidget(save);
    layout->addLayout(buttons);
}

void WizardDialog::generate() {
    const QString req = m_request->toPlainText().trimmed();
    if (req.isEmpty() || m_generated) return;
    m_generated = true;
    QJsonArray messages;
    messages.append(QJsonObject{{QStringLiteral("role"), QStringLiteral("user")},
                                {QStringLiteral("content"), META + QStringLiteral("\n\n") + req}});
    connect(m_api, &Api::completionReady, this, [this](const QString &content) {
        // parse the JSON answer (strip fences)
        QString c = content.trimmed();
        c.remove(QRegularExpression(QStringLiteral("^```(?:json)?\\s*"), QRegularExpression::CaseInsensitiveOption));
        c.remove(QRegularExpression(QStringLiteral("```\\s*$")));
        const auto doc = QJsonDocument::fromJson(c.toUtf8());
        if (doc.isObject()) {
            m_description->setText(doc.object().value(QStringLiteral("description")).toString());
            m_draft->setPlainText(doc.object().value(QStringLiteral("content")).toString());
        } else {
            m_draft->setPlainText(content.trimmed());
        }
    });
    m_api->complete(messages, m_model);
}

void WizardDialog::save() {
    const QString name = m_name->text().trimmed();
    const QString content = m_draft->toPlainText().trimmed();
    if (name.isEmpty() || content.isEmpty()) return;
    emit presetCreated(name, m_description->text().trimmed(), content);
    accept();
}
