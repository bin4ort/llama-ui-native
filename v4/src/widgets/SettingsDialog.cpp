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
    {"temperature", "Temperature", "input", false,
     "Controls the randomness of the generated text by affecting the probability distribution of the output tokens. Higher = more random, lower = more focused."},
    {"dynatemp_range", "Dynamic temperature range", "input", false,
     "The extent to which the temperature is randomly changed during generation."},
    {"dynatemp_exponent", "Dynamic temperature exponent", "input", false,
     "Modifies the temperature based on the distribution of the tokens."},
    {"top_k", "Top K", "input", true,
     "Keeps only k top tokens."},
    {"top_p", "Top P", "input", false,
     "Limits tokens based on the cumulative probability of the tokens."},
    {"min_p", "Min P", "input", false,
     "Limits tokens based on the minimum probability for a token to be considered, relative to the probability of the most likely token."},
    {"xtc_probability", "XTC probability", "input", false,
     "XTC sampler cuts out top tokens; this parameter controls the chance of cutting tokens at all. 0 disables XTC."},
    {"xtc_threshold", "XTC threshold", "input", false,
     "XTC sampler cuts out tokens until the total probability falls below this threshold."},
    {"typ_p", "Typical P", "input", false,
     "Sorts and limits tokens based on the difference between log-probability and entropy."},
    {"max_tokens", "Max tokens", "input", false,
     "The maximum number of tokens to generate."},
    {"samplers", "Samplers", "input", false,
     "The order of the samplers."},
    {"backend_sampling", "Backend sampling", "checkbox", false,
     "Sample tokens on the backend instead of the client."},
};
const QList<SettingField> PENALTIES = {
    {"repeat_last_n", "Repeat last N", "input", false,
     "How many tokens to look back to check for repetition."},
    {"repeat_penalty", "Repeat penalty", "input", false,
     "Controls the repetition of token sequences in the generated text."},
    {"presence_penalty", "Presence penalty", "input", false,
     "Limits tokens based on whether they appear in the output or not."},
    {"frequency_penalty", "Frequency penalty", "input", false,
     "Limits tokens based on how often they appear in the output."},
    {"dry_multiplier", "DRY multiplier", "input", false,
     "Multiplier for the DRY sampler."},
    {"dry_base", "DRY base", "input", false,
     "Base value for the DRY sampler."},
    {"dry_allowed_length", "DRY allowed length", "input", false,
     "Allowed length for the DRY sampler."},
    {"dry_penalty_last_n", "DRY penalty last N", "input", false,
     "How many tokens to look back for the DRY penalty."},
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
    {"customJson", "Custom JSON", "textarea"},
    {"customCss", "Custom CSS", "textarea"},
    {"logLevel", "Log level", "loglevel"},
};
} // namespace

SettingsDialog::SettingsDialog(QJsonObject cfg, const QJsonObject &props, QWidget *parent)
    : QDialog(parent), m_cfg(std::move(cfg)), m_props(props) {
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

    // v2 SettingsChatDesktopSidebar: w-64 nav at x~85 (after the app
    // strip), icon + label, active accent state, version footer
    auto *nav = new QWidget(this);
    nav->setObjectName(QStringLiteral("settingsNav"));
    nav->setFixedWidth(256);
    nav->setContentsMargins(0, 0, 0, 0);
    auto *navPad = new QWidget(this);
    navPad->setFixedWidth(85); // app strip room (v2: ml-2 + w-12 + gap)
    navPad->setAutoFillBackground(false);
    auto *nl = new QVBoxLayout(nav);
    nl->setContentsMargins(12, 36, 12, 12);
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
        item->setSizeHint(QSize(0, 40)); // v2: py-2 item ~40px pitch
        m_navList->addItem(item);
    }
    nl->addSpacing(16);
    nl->addWidget(m_navList, 1);

    // version footer (v2: "Llama UI Native vX (build Y)")
    auto *ver = new QLabel(QStringLiteral("Llama UI Native v0.6.0"), nav);
    ver->setObjectName(QStringLiteral("settingsVersion"));
    nl->addWidget(ver);
    layout->addWidget(navPad);
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
    center->setMaximumWidth(840); // v2: measured card span x400-1240
    center->setMinimumWidth(840);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(0, 112, 0, 0); // v2: md:pt-28
    cl->setSpacing(24);                    // v2: space-y-6 between cards

    // v2 section header: icon + title + bottom border (pb-6)
    auto *headerRow = new QWidget(center);
    headerRow->setObjectName(QStringLiteral("settingsSectionHeader"));
    auto *hrl = new QHBoxLayout(headerRow);
    hrl->setContentsMargins(0, 0, 0, 0);
    hrl->setSpacing(8);
    auto *secIcon = new QLabel(headerRow);
    secIcon->setPixmap(Icons::get(QStringLiteral("sliders-horizontal"), Icons::color())
                           .pixmap(18, 18));
    hrl->addWidget(secIcon);
    auto *heading = new QLabel(title, headerRow);
    heading->setObjectName(QStringLiteral("settingsSectionTitle"));
    hrl->addWidget(heading);
    hrl->addStretch(1);
    cl->addWidget(headerRow);
    cl->addSpacing(16); // v2: mb-6 after the group title

    auto addCard = [&](const QStringList &labels, const QList<QWidget *> &controls,
                       const QStringList &helps = {}) {
        auto *card = new QWidget(center);
        card->setObjectName(QStringLiteral("settingsCard"));
        card->setAttribute(Qt::WA_StyledBackground);
        auto *cardL = new QVBoxLayout(card);
        cardL->setContentsMargins(16, 16, 16, 16);
        cardL->setSpacing(6);
        for (int i = 0; i < controls.size(); ++i) {
            // v2 SettingsChatFields: the label row sits ABOVE the control
            auto *lbl = new QLabel(labels.at(i), card);
            lbl->setObjectName(QStringLiteral("settingsFieldLabel"));
            cardL->addWidget(lbl);
            // control row: full width + reset-to-default button
            auto *crow = new QHBoxLayout;
            crow->setContentsMargins(0, 0, 0, 0);
            crow->setSpacing(8);
            controls.at(i)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            controls.at(i)->setMinimumWidth(200);
            crow->addWidget(controls.at(i), 1);
            auto *resetBtn = new QPushButton(card);
            resetBtn->setProperty("ghost", true);
            resetBtn->setIcon(Icons::get(QStringLiteral("rotate-ccw"), Icons::color()));
            resetBtn->setIconSize(QSize(14, 14));
            resetBtn->setToolTip(I18n::t(QStringLiteral("Reset to default")));
            connect(resetBtn, &QPushButton::clicked, this, [this, controls, i]() {
                if (auto *le = qobject_cast<QLineEdit *>(controls.at(i))) {
                    le->clear();
                } else if (auto *te = qobject_cast<QPlainTextEdit *>(controls.at(i))) {
                    te->clear();
                } else if (auto *cb = qobject_cast<QComboBox *>(controls.at(i))) {
                    cb->setCurrentIndex(0);
                }
            });
            crow->addWidget(resetBtn);
            cardL->addLayout(crow);
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
            // v2: the input stays empty and shows the server default as
            // the placeholder ('Default: 0.8'), with a reset button
            QString value = m_cfg.value(f.key).toString();
            QString defaultText = QStringLiteral("Default");
            const QJsonValue serverVal = m_props.value(f.key);
            if (!serverVal.isUndefined() && !serverVal.isNull()) {
                QString dv;
                if (serverVal.isDouble()) {
                    dv = QString::number(serverVal.toDouble(), 'f', 6);
                    while (dv.contains(QLatin1Char('.')) && dv.endsWith(QLatin1Char('0')))
                        dv.chop(1);
                    if (dv.endsWith(QLatin1Char('.'))) dv.chop(1);
                } else {
                    dv = serverVal.toVariant().toString();
                }
                if (!dv.isEmpty()) defaultText = QStringLiteral("Default: ") + dv;
            }
            auto *le = new QLineEdit(value);
            le->setPlaceholderText(defaultText);
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
            // v2 renders Language and Theme as separate cards
            addCard(labels, controls, {QStringLiteral("Choose the interface language.")});
            labels.clear();
            controls.clear();
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
        if (!controls.isEmpty()) addCard(labels, controls);
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
    // v2 SettingsChatImportExportTab: group cards with title + description
    // + action button (Conversations: export/import/delete-all; Settings:
    // export/import JSON)
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    auto *v = new QVBoxLayout(host);
    v->setContentsMargins(0, 112, 0, 0); // v2: md:pt-28
    v->setSpacing(24);

    auto *center = new QWidget(host);
    center->setObjectName(QStringLiteral("settingsCenter"));
    center->setMaximumWidth(840);
    center->setMinimumWidth(840);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(0, 0, 0, 0);
    cl->setSpacing(24);

    auto *heading = new QLabel(I18n::t(QStringLiteral("Import/Export")), center);
    heading->setObjectName(QStringLiteral("settingsSectionTitle"));
    cl->addWidget(heading);
    cl->addSpacing(8);

    auto addSection = [&](const QString &title, const QString &desc,
                          const QString &buttonText, const QString &icon,
                          std::function<void()> fn, bool destructive = false) {
        auto *card = new QWidget(center);
        card->setObjectName(QStringLiteral("settingsCard"));
        card->setAttribute(Qt::WA_StyledBackground);
        auto *cl2 = new QVBoxLayout(card);
        cl2->setContentsMargins(16, 16, 16, 16);
        cl2->setSpacing(8);
        auto *row = new QHBoxLayout;
        auto *iconLbl = new QLabel(card);
        iconLbl->setPixmap(Icons::get(icon, Icons::color()).pixmap(16, 16));
        row->addWidget(iconLbl);
        auto *t = new QLabel(title, card);
        t->setObjectName(QStringLiteral("settingsFieldLabel"));
        row->addWidget(t);
        row->addStretch(1);
        auto *btn = new QPushButton(buttonText, card);
        btn->setProperty("outline", true);
        if (destructive) btn->setProperty("destructive", true);
        connect(btn, &QPushButton::clicked, this, fn);
        row->addWidget(btn);
        cl2->addLayout(row);
        auto *d = new QLabel(desc, card);
        d->setObjectName(QStringLiteral("settingsHelp"));
        d->setWordWrap(true);
        cl2->addWidget(d);
        cl->addWidget(card);
    };

    auto *convHeading = new QLabel(I18n::t(QStringLiteral("Conversations")), center);
    convHeading->setObjectName(QStringLiteral("settingsGroupTitle"));
    cl->addWidget(convHeading);

    addSection(I18n::t(QStringLiteral("Export")),
               I18n::t(QStringLiteral("Download your conversations as a JSON file with all messages.")),
               I18n::t(QStringLiteral("Export conversations")), QStringLiteral("download"),
               [this]() {
                   QFileDialog::getSaveFileName(this, I18n::t(QStringLiteral("Export conversations")),
                                                QStringLiteral("conversations.json"));
               });
    addSection(I18n::t(QStringLiteral("Import")),
               I18n::t(QStringLiteral("Import conversations from a previously exported file. This will merge with your existing conversations.")),
               I18n::t(QStringLiteral("Import conversations")), QStringLiteral("upload"),
               []() {});
    addSection(I18n::t(QStringLiteral("Delete All")),
               I18n::t(QStringLiteral("Permanently delete all conversations and their messages. This action cannot be undone.")),
               I18n::t(QStringLiteral("Delete all conversations")), QStringLiteral("trash-2"),
               [this]() {
                   const auto ret = QMessageBox::question(
                       this, I18n::t(QStringLiteral("Delete all conversations")),
                       I18n::t(QStringLiteral("Are you sure?")));
                   if (ret == QMessageBox::Yes) emit clearAllConversationsRequested();
               }, true);

    auto *setHeading = new QLabel(I18n::t(QStringLiteral("Settings")), center);
    setHeading->setObjectName(QStringLiteral("settingsGroupTitle"));
    cl->addWidget(setHeading);

    addSection(I18n::t(QStringLiteral("Export")),
               I18n::t(QStringLiteral("Export your chat settings and preferences as a JSON file.")),
               I18n::t(QStringLiteral("Export settings")), QStringLiteral("download"),
               [this]() {
                   const QString path = QFileDialog::getSaveFileName(
                       this, I18n::t(QStringLiteral("Export settings")),
                       QStringLiteral("llama-ui-settings.json"));
                   if (!path.isEmpty()) {
                       QFile f(path);
                       if (f.open(QIODevice::WriteOnly))
                           f.write(QJsonDocument(m_cfg).toJson(QJsonDocument::Indented));
                   }
               });
    addSection(I18n::t(QStringLiteral("Import")),
               I18n::t(QStringLiteral("Import chat settings from a previously exported JSON file. This will merge with your existing settings.")),
               I18n::t(QStringLiteral("Import settings")), QStringLiteral("upload"),
               [this]() {
                   const QString path = QFileDialog::getOpenFileName(
                       this, I18n::t(QStringLiteral("Import settings")),
                       QString(), QStringLiteral("JSON (*.json)"));
                   if (path.isEmpty()) return;
                   QFile f(path);
                   if (!f.open(QIODevice::ReadOnly)) return;
                   const auto doc = QJsonDocument::fromJson(f.readAll());
                   if (doc.isObject()) {
                       QJsonObject merged = m_cfg;
                       const QJsonObject src = doc.object();
                       for (auto it = src.begin(); it != src.end(); ++it)
                           merged.insert(it.key(), it.value());
                       m_cfg = merged;
                       emit configChanged(m_cfg);
                       accept();
                   }
               });

    cl->addStretch(1);
    v->addWidget(center, 0, Qt::AlignHCenter);
    scroll->setWidget(host);
    return scroll;
}

void SettingsDialog::setSection(int index) {

    if (m_navList && index >= 0 && index < m_navList->count())
        m_navList->setCurrentRow(index);
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
