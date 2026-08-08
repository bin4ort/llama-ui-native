#include "SettingsDialog.h"
#include "../i18n.h"
#include "../Theme.h"
#include "../Config.h"
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>

namespace {
// Registry tables (UI-SPEC §09 / §2 labels verbatim).
const QList<SettingField> GENERAL = {
    {"sendOnEnter", "Send message on Enter", "checkbox"},
    {"autoMicOnEmpty", "Show microphone on empty input", "checkbox"},
    {"enableContinueGeneration", "Enable \"Continue\" button", "checkbox"},
    {"pasteLongTextToFileLen", "Paste long text to file length", "input", true},
    {"copyTextAttachmentsAsPlainText", "Copy text attachments as plain text", "checkbox"},
    {"pdfAsImage", "Parse PDF as image", "checkbox"},
    {"maxImageMPixels", "Maximum image resolution (megapixels)", "input"},
    {"webUiAddress", "Web UI Address", "input"},
    {"webUiEnabled", "Enable Web UI", "checkbox"},
    {"apiKey", "API Key", "input"},
    {"systemMessage", "Default system prompt", "textarea"},
};
const QList<SettingField> DISPLAY = {
    {"showMessageStats", "Show message generation statistics", "checkbox"},
    {"showAgenticTurnStats", "Show statistics for individual agentic turns", "checkbox"},
    {"showThoughtInProgress", "Show thought in progress", "checkbox"},
    {"alwaysShowToolCallContent", "Always show tool call content", "checkbox"},
    {"renderUserContentAsMarkdown", "Render user content as Markdown", "checkbox"},
    {"renderThinkingAsMarkdown", "Render thinking as Markdown", "checkbox"},
    {"fullHeightCodeBlocks", "Use full height code blocks", "checkbox"},
    {"disableAutoScroll", "Disable automatic scroll", "checkbox"},
    {"alwaysShowSidebarOnDesktop", "Always show sidebar on desktop", "checkbox"},
    {"showRawModelNames", "Show raw model names", "checkbox"},
    {"showModelQuantization", "Show model quantization information", "checkbox"},
    {"showModelTags", "Show model tags", "checkbox"},
    {"customJson", "Custom JSON", "textarea"},
    {"customCss", "Custom CSS", "textarea"},
};
const QList<SettingField> SAMPLING = {
    {"temperature", "Temperature", "input"},
    {"dynatemp_range", "Dynamic temperature range", "input"},
    {"dynatemp_exponent", "Dynamic temperature exponent", "input"},
    {"top_k", "Top K", "input", true},
    {"top_p", "Top P", "input"},
    {"min_p", "Min P", "input"},
    {"xtc_probability", "XTC probability", "input"},
    {"xtc_threshold", "XTC threshold", "input"},
    {"typ_p", "Typical P", "input"},
    {"max_tokens", "Max tokens", "input"},
    {"samplers", "Samplers", "input"},
    {"backend_sampling", "Backend sampling", "checkbox"},
};
const QList<SettingField> PENALTIES = {
    {"repeat_last_n", "Repeat last N", "input"},
    {"repeat_penalty", "Repeat penalty", "input"},
    {"presence_penalty", "Presence penalty", "input"},
    {"frequency_penalty", "Frequency penalty", "input"},
    {"dry_multiplier", "DRY multiplier", "input"},
    {"dry_base", "DRY base", "input"},
    {"dry_allowed_length", "DRY allowed length", "input"},
    {"dry_penalty_last_n", "DRY penalty last N", "input"},
};
const QList<SettingField> TOOLS = {
    {"toolCalculateEnabled", "Calculator", "checkbox"},
    {"toolFetchUrlEnabled", "Fetch URL", "checkbox"},
    {"toolToTableEnabled", "Table from data", "checkbox"},
    {"toolJsonEnabled", "JSON helper", "checkbox"},
    {"toolClipboardEnabled", "Clipboard", "checkbox"},
    {"toolNotifyEnabled", "Notifications", "checkbox"},
    {"toolTodoEnabled", "Task list", "checkbox"},
    {"toolWeatherEnabled", "Weather", "checkbox"},
    {"toolWikipediaEnabled", "Wikipedia lookup", "checkbox"},
    {"toolPlotChartEnabled", "Chart", "checkbox"},
    {"presetToolsEnabled", "Prompt preset switching tools", "checkbox"},
};
const QList<SettingField> AGENTIC = {
    {"agenticMaxTurns", "Agentic turns", "input", true},
    {"mcpRequestTimeoutSeconds", "MCP request timeout (seconds)", "input", true},
};
const QList<SettingField> DEVELOPER = {
    {"preEncodeConversation", "Pre-fill KV cache after response", "checkbox"},
    {"disableReasoningParsing", "Disable reasoning content parsing", "checkbox"},
    {"excludeReasoningFromContext", "Exclude reasoning from context", "checkbox"},
    {"showRawOutputSwitch", "Enable raw output toggle", "checkbox"},
    {"jsSandboxEnabled", "JavaScript sandbox tool", "checkbox"},
    {"symbolicMathEnabled", "Symbolic math (nerdamer)", "checkbox"},
    {"serverEndpoint", "Server endpoint", "input"},
    {"logLevel", "Log level", "loglevel"},
};
} // namespace

SettingsDialog::SettingsDialog(QJsonObject cfg, QWidget *parent)
    : QDialog(parent), m_cfg(std::move(cfg)) {
    setObjectName(QStringLiteral("appDialog"));
    setWindowTitle(QStringLiteral("Settings"));
    setModal(true);
    resize(560, 640);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(12);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(buildSection(QStringLiteral("General"), GENERAL, true, true), QStringLiteral("General"));
    m_tabs->addTab(buildSection(QStringLiteral("Display"), DISPLAY), QStringLiteral("Display"));
    m_tabs->addTab(buildSection(QStringLiteral("Sampling"), SAMPLING), QStringLiteral("Sampling"));
    m_tabs->addTab(buildSection(QStringLiteral("Penalties"), PENALTIES), QStringLiteral("Penalties"));
    m_tabs->addTab(buildSection(QStringLiteral("Agentic"), AGENTIC), QStringLiteral("Agentic"));
    m_tabs->addTab(buildSection(QStringLiteral("Developer"), DEVELOPER), QStringLiteral("Developer"));
    m_tabs->addTab(buildSection(QStringLiteral("Tools"), TOOLS), QStringLiteral("Tools"));
    m_tabs->addTab(buildImportExport(), I18n::t(QStringLiteral("Import/Export")));
    layout->addWidget(m_tabs, 1);

    // footer: reset-to-default + cancel/save (UI-SPEC §9.4)
    auto *buttons = new QHBoxLayout;
    auto *reset = new QPushButton(I18n::t(QStringLiteral("Reset to default")));
    reset->setProperty("outline", true);
    connect(reset, &QPushButton::clicked, this, [this]() {
        const auto ret = QMessageBox::question(
            this, I18n::t(QStringLiteral("Reset Settings to Default")),
            I18n::t(QStringLiteral("Are you sure?")));
        if (ret != QMessageBox::Yes) return;
        QFile::remove(Config::path());
        m_cfg = Config::load(); // fresh defaults (with merged file)
        Config::save(m_cfg);
        emit configChanged(m_cfg);
        accept();
    });
    buttons->addWidget(reset);
    buttons->addStretch(1);
    auto *cancel = new QPushButton(I18n::t(QStringLiteral("Cancel")));
    cancel->setProperty("outline", true);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancel);
    auto *save = new QPushButton(I18n::t(QStringLiteral("Save settings")));
    connect(save, &QPushButton::clicked, this, &SettingsDialog::apply);
    buttons->addWidget(save);
    layout->addLayout(buttons);
}

QWidget *SettingsDialog::buildSection(const QString &title, const QList<SettingField> &fields,
                                      bool useComboLang, bool useComboTheme) {
    Q_UNUSED(title);
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    auto *form = new QFormLayout(host);
    form->setContentsMargins(12, 12, 12, 12);
    form->setSpacing(10);

    if (useComboLang) {
        // v2 human-readable language labels
        const QMap<QString, QString> langNames{
            {QStringLiteral("en"), QStringLiteral("English")},
            {QStringLiteral("de"), QStringLiteral("Deutsch")},
            {QStringLiteral("ru"), QStringLiteral("Русский")},
            {QStringLiteral("es"), QStringLiteral("Español")},
            {QStringLiteral("fr"), QStringLiteral("Français")},
            {QStringLiteral("zh"), QStringLiteral("中文")},
            {QStringLiteral("ja"), QStringLiteral("日本語")},
            {QStringLiteral("ko"), QStringLiteral("한국어")},
            {QStringLiteral("pt"), QStringLiteral("Português")},
            {QStringLiteral("it"), QStringLiteral("Italiano")},
            {QStringLiteral("tr"), QStringLiteral("Türkçe")},
            {QStringLiteral("pl"), QStringLiteral("Polski")},
        };
        auto *lang = new QComboBox;
        for (const auto &code : I18n::languages())
            lang->addItem(langNames.value(code, code), code);
        lang->setCurrentIndex(qMax(0, lang->findData(m_cfg.value(QStringLiteral("language")).toString())));
        form->addRow(QStringLiteral("Language"), lang);
        m_controls.append({QStringLiteral("language"), lang});
    }
    if (useComboTheme) {
        // v2 theme names (settings-registry order)
        const QList<QPair<QString, QString>> themes{
            {QStringLiteral("System"), QStringLiteral("system")},
            {QStringLiteral("Light"), QStringLiteral("light")},
            {QStringLiteral("Snow"), QStringLiteral("snow")},
            {QStringLiteral("Dark"), QStringLiteral("dark")},
            {QStringLiteral("AMOLED"), QStringLiteral("amoled")},
            {QStringLiteral("Nord"), QStringLiteral("nord")},
            {QStringLiteral("Dracula"), QStringLiteral("dracula")},
            {QStringLiteral("Cobalt"), QStringLiteral("cobalt")},
            {QStringLiteral("Solarized Dark"), QStringLiteral("solarized")},
            {QStringLiteral("Gruvbox Dark"), QStringLiteral("gruvbox")},
            {QStringLiteral("Gruvbox Light"), QStringLiteral("gruvbox-light")},
            {QStringLiteral("Ionized Purple"), QStringLiteral("ionized-purple")},
            {QStringLiteral("Ionized Red"), QStringLiteral("ionized-red")},
            {QStringLiteral("Ionized Cyan"), QStringLiteral("ionized-cyan")},
        };
        auto *theme = new QComboBox;
        for (const auto &[label, id] : themes)
            theme->addItem(label, id);
        theme->setCurrentIndex(qMax(0, theme->findData(m_cfg.value(QStringLiteral("theme")).toString())));
        form->addRow(QStringLiteral("Theme"), theme);
        m_controls.append({QStringLiteral("theme"), theme});
    }

    for (const auto &f : fields) {
        QWidget *control = nullptr;
        if (f.type == QStringLiteral("checkbox")) {
            auto *cb = new QCheckBox;
            cb->setChecked(m_cfg.value(f.key).toBool(false));
            control = cb;
        } else if (f.type == QStringLiteral("textarea")) {
            auto *te = new QPlainTextEdit(m_cfg.value(f.key).toString());
            te->setMinimumHeight(100);
            te->setMaximumHeight(200);
            control = te;
        } else if (f.type == QStringLiteral("loglevel")) {
            auto *combo = new QComboBox;
            combo->addItem(I18n::t(QStringLiteral("Errors only")), 0);
            combo->addItem(I18n::t(QStringLiteral("Warnings")), 1);
            combo->addItem(I18n::t(QStringLiteral("Info")), 2);
            combo->addItem(I18n::t(QStringLiteral("Debug")), 3);
            combo->addItem(I18n::t(QStringLiteral("Trace")), 4);
            const int lvl = m_cfg.value(f.key).toInt(2);
            combo->setCurrentIndex(qBound(0, lvl, 4));
            control = combo;
        } else {
            auto *le = new QLineEdit(m_cfg.value(f.key).toString());
            le->setPlaceholderText(QStringLiteral("Default"));
            if (f.positiveInt) le->setValidator(new QIntValidator(0, 1 << 30, le));
            control = le;
        }
        form->addRow(f.label, control);
        m_controls.append({f.key, control});
    }
    scroll->setWidget(host);
    return scroll;
}

QWidget *SettingsDialog::buildImportExport() {
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    auto *v = new QVBoxLayout(host);
    v->setContentsMargins(12, 12, 12, 12);
    v->setSpacing(10);

    auto *jsonText = new QPlainTextEdit;
    jsonText->setPlainText(QJsonDocument(m_cfg).toJson(QJsonDocument::Indented));
    jsonText->setMinimumHeight(280);
    v->addWidget(jsonText);

    auto *row = new QHBoxLayout;
    auto *copyBtn = new QPushButton(I18n::t(QStringLiteral("Copy")));
    copyBtn->setProperty("outline", true);
    connect(copyBtn, &QPushButton::clicked, this, [jsonText]() {
        QApplication::clipboard()->setText(jsonText->toPlainText());
    });
    row->addWidget(copyBtn);
    auto *importBtn = new QPushButton(I18n::t(QStringLiteral("Import")));
    importBtn->setProperty("outline", true);
    connect(importBtn, &QPushButton::clicked, this, [this, jsonText]() {
        const auto doc = QJsonDocument::fromJson(jsonText->toPlainText().toUtf8());
        if (!doc.isObject()) {
            QMessageBox::warning(this, QStringLiteral("Import"),
                                 I18n::t(QStringLiteral("Invalid JSON — settings not imported.")));
            return;
        }
        m_cfg = doc.object();
        emit configChanged(m_cfg);
        accept();
    });
    row->addWidget(importBtn);
    auto *downloadBtn = new QPushButton(I18n::t(QStringLiteral("Download")));
    downloadBtn->setProperty("outline", true);
    connect(downloadBtn, &QPushButton::clicked, this, [this, jsonText]() {
        const QString path = QFileDialog::getSaveFileName(
            this, I18n::t(QStringLiteral("Export settings")),
            QStringLiteral("llama-ui-settings.json"));
        if (!path.isEmpty()) {
            QFile f(path);
            if (f.open(QIODevice::WriteOnly))
                f.write(jsonText->toPlainText().toUtf8());
        }
    });
    row->addWidget(downloadBtn);
    row->addStretch(1);
    v->addLayout(row);

    scroll->setWidget(host);
    return scroll;
}

void SettingsDialog::apply() {
    for (const auto &[key, w] : m_controls) {
        if (auto *cb = qobject_cast<QCheckBox *>(w)) {
            m_cfg.insert(key, cb->isChecked());
        } else if (auto *combo = qobject_cast<QComboBox *>(w)) {
            m_cfg.insert(key, QJsonValue::fromVariant(combo->currentData()));
        } else if (auto *te = qobject_cast<QPlainTextEdit *>(w)) {
            m_cfg.insert(key, te->toPlainText().trimmed());
        } else if (auto *le = qobject_cast<QLineEdit *>(w)) {
            m_cfg.insert(key, le->text().trimmed());
        }
    }
    emit configChanged(m_cfg);
    accept();
}
