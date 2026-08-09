#include "SettingsDialog.h"
#include "../Icons.h"
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
#include <QLabel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QListWidget>
#include <QSize>
#include <QVBoxLayout>

namespace {
// Registry tables (UI-SPEC §09 / §2 labels verbatim).
const QList<SettingField> GENERAL = {
    {"serverEndpoint", "Server Endpoint", "input", false,
     "Set the llama.cpp server or OpenAI-compatible API endpoint."},
    {"webUiAddress", "Web UI Address", "input", false,
     "The address where the Web UI is hosted."},
    {"webUiEnabled", "Enable Web UI", "checkbox", false,
     "Allow access to the Web UI from other devices."},
    {"apiKey", "API Key", "input", false,
     "Set the API Key if you are using the --api-key option for the server."},
    {"systemMessage", "Default system prompt", "textarea", false,
     "The default system prompt — the starting message that defines how the model should behave."},
    {"pasteLongTextToFileLen", "Paste long text to file length", "input", true,
     "Paste long text as a file attachment instead of inline."},
    {"sendOnEnter", "Send message on Enter", "checkbox", false,
     "Use Enter to send messages and Shift + Enter for new lines."},
    {"autoMicOnEmpty", "Show microphone on empty input", "checkbox", false,
     "Show the microphone button when the input is empty."},
    {"enableContinueGeneration", "Enable \"Continue\" button", "checkbox", false,
     "Show a button to continue the assistant response."},
    {"titleGenerationMode", "Conversation title", "titleradio", false,
     "How conversation titles are generated."},
    {"titleGenerationPrompt", "LLM title generation prompt", "textarea", false,
     "The prompt used to generate conversation titles with the LLM."},
    {"copyTextAttachmentsAsPlainText", "Copy text attachments as plain text", "checkbox", false,
     "Copy text attachments as plain text instead of files."},
    {"pdfAsImage", "Parse PDF as image", "checkbox", false,
     "Parse PDF files as images instead of text."},
    {"maxImageMPixels", "Maximum image resolution (megapixels)", "input", false,
     "Maximum resolution of uploaded images."},
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
    {"logLevel", "Log level", "loglevel"},
};
} // namespace

SettingsDialog::SettingsDialog(QJsonObject cfg, QWidget *parent)
    : QDialog(parent), m_cfg(std::move(cfg)) {
    setObjectName(QStringLiteral("appDialog"));
    setWindowTitle(QStringLiteral("Settings"));
    setModal(true);
    resize(1400, 900);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);
    auto *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // v2 SettingsChatDesktopSidebar: w-64 nav with icon + label, active
    // accent state, version footer
    auto *nav = new QWidget(this);
    nav->setObjectName(QStringLiteral("settingsNav"));
    nav->setFixedWidth(256);
    auto *nl = new QVBoxLayout(nav);
    nl->setContentsMargins(12, 16, 12, 12);
    nl->setSpacing(4);

    auto *heading = new QLabel(I18n::t(QStringLiteral("Settings")), nav);
    heading->setObjectName(QStringLiteral("settingsHeading"));
    heading->setPixmap(Icons::get(QStringLiteral("settings"), Icons::color()).pixmap(18, 18));
    nl->addWidget(heading);
    nl->addSpacing(8);

    m_navList = new QListWidget(nav);
    m_navList->setObjectName(QStringLiteral("settingsNavList"));
    m_navList->setFrameShape(QFrame::NoFrame);
    const QStringList sections = {
        QStringLiteral("General"), QStringLiteral("Display"), QStringLiteral("Sampling"),
        QStringLiteral("Penalties"), QStringLiteral("Agentic"), QStringLiteral("Developer"),
        QStringLiteral("Tools"), QStringLiteral("Import/Export")};
    const QStringList icons = {
        QStringLiteral("sliders-horizontal"), QStringLiteral("eye"), QStringLiteral("gauge"),
        QStringLiteral("triangle-alert"), QStringLiteral("bot"), QStringLiteral("code-2"),
        QStringLiteral("wrench"), QStringLiteral("arrow-left-right")};
    for (int i = 0; i < sections.size(); ++i) {
        auto *item = new QListWidgetItem(sections.at(i));
        item->setIcon(Icons::get(icons.at(i), Icons::color()));
        m_navList->addItem(item);
    }
    nl->addWidget(m_navList, 1);

    // version footer (v2: "Llama UI Native vX (build Y)")
    auto *ver = new QLabel(QStringLiteral("Llama UI Native v0.6.0"), nav);
    ver->setObjectName(QStringLiteral("settingsVersion"));
    nl->addWidget(ver);
    layout->addWidget(nav);

    // content stack (v2: the section renders on the right)
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName(QStringLiteral("settingsStack"));
    m_stack->addWidget(buildSection(QStringLiteral("General"), GENERAL, true, true));
    m_stack->addWidget(buildSection(QStringLiteral("Display"), DISPLAY));
    m_stack->addWidget(buildSection(QStringLiteral("Sampling"), SAMPLING));
    m_stack->addWidget(buildSection(QStringLiteral("Penalties"), PENALTIES));
    m_stack->addWidget(buildSection(QStringLiteral("Agentic"), AGENTIC));
    m_stack->addWidget(buildSection(QStringLiteral("Developer"), DEVELOPER));
    m_stack->addWidget(buildSection(QStringLiteral("Tools"), TOOLS));
    m_stack->addWidget(buildImportExport());

    auto *rightCol = new QVBoxLayout;
    rightCol->setContentsMargins(0, 0, 0, 0);
    rightCol->setSpacing(0);
    rightCol->addWidget(m_stack, 1);
    layout->addLayout(rightCol, 1);

    // footer: reset-to-default + cancel/save (v2 SettingsFooter, inside
    // the content pane, sticky bottom)
    auto *buttons = new QWidget(this);
    buttons->setObjectName(QStringLiteral("settingsFooter"));
    buttons->setAttribute(Qt::WA_StyledBackground);
    buttons->setAutoFillBackground(true);
    buttons->setMinimumHeight(55);
    // v2 footer: buttons grouped and centered in the content pane
    auto *bl = new QHBoxLayout(buttons);
    bl->setContentsMargins(24, 10, 24, 10);
    auto *group = new QWidget(buttons);
    group->setObjectName(QStringLiteral("settingsFooterGroup"));
    group->setMinimumWidth(780);
    group->setMaximumWidth(780);
    auto *gl = new QHBoxLayout(group);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setSpacing(8);
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
    gl->addWidget(reset);
    gl->addStretch(1);
    auto *save = new QPushButton(I18n::t(QStringLiteral("Save settings")));
    connect(save, &QPushButton::clicked, this, &SettingsDialog::apply);
    gl->addWidget(save);
    bl->addWidget(group, 0, Qt::AlignHCenter);

    rightCol->addWidget(buttons);
    outer->addLayout(layout, 1);

    connect(m_navList, &QListWidget::currentRowChanged, this,
            [this](int row) { m_stack->setCurrentIndex(qMax(0, row)); });
    m_navList->setCurrentRow(0);
}

QWidget *SettingsDialog::buildSection(const QString &title, const QList<SettingField> &fields,
                                      bool useComboLang, bool useComboTheme) {
    // v2 section page: h1 heading + centered 672px column of cards, each
    // card holding labeled field rows
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    auto *col = new QVBoxLayout(host);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(12);
    col->addStretch(0);

    auto *center = new QWidget(host);
    center->setObjectName(QStringLiteral("settingsCenter"));
    center->setMaximumWidth(672);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(8);

    auto *heading = new QLabel(title, center);
    heading->setObjectName(QStringLiteral("settingsSectionTitle"));
    cl->addWidget(heading);
    cl->addSpacing(4);

    auto addCard = [&](const QStringList &labels, const QList<QWidget *> &controls,
                       const QStringList &helps = {}) {
        auto *card = new QWidget(center);
        card->setObjectName(QStringLiteral("settingsCard"));
        auto *cardL = new QVBoxLayout(card);
        cardL->setContentsMargins(16, 12, 16, 12);
        cardL->setSpacing(4);
        for (int i = 0; i < controls.size(); ++i) {
            auto *row = new QHBoxLayout;
            auto *lbl = new QLabel(labels.at(i), card);
            lbl->setObjectName(QStringLiteral("settingsFieldLabel"));
            row->addWidget(lbl, 1);
            row->addWidget(controls.at(i), 2);
            cardL->addLayout(row);
            if (i < helps.size() && !helps.at(i).isEmpty()) {
                auto *help = new QLabel(helps.at(i), card);
                help->setObjectName(QStringLiteral("settingsHelp"));
                help->setWordWrap(true);
                cardL->addWidget(help);
            }
        }
        cl->addWidget(card);
    };

    auto makeControl = [&](const SettingField &f) -> QWidget * {
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
        } else if (f.type == QStringLiteral("titleradio")) {
            // v2 'Conversation title': First line / LLM
            auto *combo = new QComboBox;
            combo->addItem(I18n::t(QStringLiteral("First line of the conversation")),
                           QStringLiteral("first_line"));
            combo->addItem(QStringLiteral("LLM"),
                           QStringLiteral("llm"));
            const QString mode = m_cfg.value(f.key).toString();
            combo->setCurrentIndex(qMax(0, combo->findData(mode)));
            control = combo;
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
        m_controls.append({f.key, control});
        return control;
    };

    // Language + Theme card (v2 General groups them first)
    if (useComboLang || useComboTheme) {
        QStringList labels;
        QList<QWidget *> controls;
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
            labels << QStringLiteral("Language");
            controls << lang;
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
            labels << QStringLiteral("Theme");
            controls << theme;
            m_controls.append({QStringLiteral("theme"), theme});
        }
        addCard(labels, controls);
    }

    // fields render in registry order (v2 interleaves checkboxes and
    // inputs); checkbox runs are bundled ~3 per card
    QStringList checkLabels;
    QStringList checkHelps;
    QList<QWidget *> checkControls;
    auto flushChecks = [&]() {
        if (!checkControls.isEmpty()) {
            addCard(checkLabels, checkControls, checkHelps);
            checkLabels.clear();
            checkControls.clear();
            checkHelps.clear();
        }
    };
    for (const auto &f : fields) {
        if (f.type == QStringLiteral("checkbox")) {
            if (checkControls.size() >= 3) flushChecks();
            checkLabels << f.label;
            checkHelps << f.help;
            checkControls << makeControl(f);
        } else {
            flushChecks();
            addCard({f.label}, {makeControl(f)}, {f.help});
        }
    }
    flushChecks();

    cl->addStretch(1);
    col->addWidget(center, 0, Qt::AlignHCenter);
    col->addStretch(1);
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
