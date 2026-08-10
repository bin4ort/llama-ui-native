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
#include <QFontMetrics>
#include <QFormLayout>
#include <cmath>
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
#include <QPainter>
#include <QSize>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

namespace {

// Nav item renderer matching the v2 SettingsChatDesktopSidebar: the selected
// pill fills the whole row (rounded 8px, accent), 16px icon at x12, 14px
// label at x48 (icon + gap-3 + ml-2).
class NavDelegate : public QStyledItemDelegate {
public:
    NavDelegate(const QColor &accent, const QColor &fg, const QColor &muted,
                QObject *parent = nullptr)
        : QStyledItemDelegate(parent), m_accent(accent), m_fg(fg), m_muted(muted) {}

    void paint(QPainter *p, const QStyleOptionViewItem &opt,
               const QModelIndex &idx) const override {
        const bool selected = opt.state & QStyle::State_Selected;
        const QRect r = opt.rect;
        if (selected) {
            p->setRenderHint(QPainter::Antialiasing);
            p->setPen(Qt::NoPen);
            p->setBrush(m_accent);
            p->drawRoundedRect(QRectF(r).adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);
        }
        const QIcon icon = idx.data(Qt::DecorationRole).value<QIcon>();
        if (!icon.isNull())
            icon.paint(p, QRect(r.left() + 12, r.center().y() - 8, 16, 16));
        QFont f = opt.font;
        f.setPixelSize(14);
        p->setFont(f);
        p->setPen(selected ? m_fg : m_muted);
        p->drawText(QRect(r.left() + 48, r.top(), r.width() - 56, r.height()),
                    Qt::AlignVCenter | Qt::AlignLeft,
                    idx.data(Qt::DisplayRole).toString());
    }

private:
    QColor m_accent, m_fg, m_muted;
};

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
    {"showMessageStats", "Show message generation statistics", "checkbox", false,
     "Display generation statistics (tokens/second, token count, duration) below each assistant message."},
    {"showThoughtInProgress", "Show thought in progress", "checkbox", false,
     "Expand thought process by default when generating messages."},
    {"alwaysShowToolCallContent", "Always show tool call content", "checkbox", false,
     "Automatically expand tool call details while executing and keep them expanded after completion."},
    {"renderUserContentAsMarkdown", "Render user content as Markdown", "checkbox", false,
     "Render user messages using markdown formatting in the chat."},
    {"renderThinkingAsMarkdown", "Render thinking as Markdown", "checkbox", false,
     "Render the reasoning/thinking block content as formatted Markdown instead of plain text."},
    {"fullHeightCodeBlocks", "Use full height code blocks", "checkbox", false,
     "Always display code blocks at their full natural height, overriding any height limits."},
    {"disableAutoScroll", "Disable automatic scroll", "checkbox", false,
     "Disable automatic scrolling while messages stream so you can control the viewport position manually."},
    {"alwaysShowSidebarOnDesktop", "Always show sidebar on desktop", "checkbox", false,
     "Always keep the sidebar visible on desktop instead of auto-hiding it."},
    {"showRawModelNames", "Show raw model names", "checkbox", false,
     "Display full raw model identifiers (e.g. \"ggml-org/GLM-4.7-Flash-GGUF:Q8_0\") instead of parsed names with badges."},
    {"showModelQuantization", "Show model quantization information", "checkbox", false,
     "Display quantization badges (e.g. Q8_0, Q4_K_M) next to model names throughout the interface."},
    {"showModelTags", "Show model tags", "checkbox", false,
     "Display model tags (e.g. \"vision\", \"reasoning\") next to model names throughout the interface."},
};
const QList<SettingField> SAMPLING = {
    {"temperature", "Temperature", "input", false,
     "Controls the randomness of the generated text by affecting the probability distribution of the output tokens. Higher = more random, lower = more focused."},
    {"dynatemp_range", "Dynamic temperature range", "input", false,
     "Addon for the temperature sampler. The added value to the range of dynamic temperature, which adjusts probabilities by entropy of tokens."},
    {"dynatemp_exponent", "Dynamic temperature exponent", "input", false,
     "Addon for the temperature sampler. Smoothes out the probability redistribution based on the most probable token."},
    {"top_k", "Top K", "input", true,
     "Keeps only k top tokens."},
    {"top_p", "Top P", "input", false,
     "Limits tokens to those that together have a cumulative probability of at least p"},
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
     "Enable backend-based samplers. When enabled, supported samplers run on the accelerator backend for faster sampling."},
};
const QList<SettingField> PENALTIES = {
    {"repeat_last_n", "Repeat last N", "input", false,
     "Last n tokens to consider for penalizing repetition"},
    {"repeat_penalty", "Repeat penalty", "input", false,
     "Controls the repetition of token sequences in the generated text"},
    {"presence_penalty", "Presence penalty", "input", false,
     "Limits tokens based on whether they appear in the output or not."},
    {"frequency_penalty", "Frequency penalty", "input", false,
     "Limits tokens based on how often they appear in the output."},
    {"dry_multiplier", "DRY multiplier", "input", false,
     "DRY sampling reduces repetition in generated text even across long contexts. This parameter sets the DRY sampling multiplier."},
    {"dry_base", "DRY base", "input", false,
     "DRY sampling reduces repetition in generated text even across long contexts. This parameter sets the DRY sampling base value."},
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
    {"agenticMaxTurns", "Agentic turns", "input", true,
     "Maximum number of tool execution cycles before stopping (prevents infinite loops)."},
    {"mcpRequestTimeoutSeconds", "MCP request timeout (seconds)", "input", true,
     "Timeout for individual MCP tool calls."},
};
const QList<SettingField> DEVELOPER = {
    {"preEncodeConversation", "Pre-fill KV cache after response", "checkbox", false,
     "After each response, re-submit the conversation to pre-fill the server KV cache. Makes the next turn faster since the prompt is already encoded while you read the response."},
    {"disableReasoningParsing", "Disable reasoning content parsing", "checkbox", false,
     "Send reasoning_format=none so the server returns thinking tokens inline instead of extracting them into a separate field."},
    {"excludeReasoningFromContext", "Exclude reasoning from context", "checkbox", false,
     "Strip thinking from previous messages before sending. When off, thinking is sent back via the reasoning_content field so the model sees its own chain-of-thought across turns."},
    {"showRawOutputSwitch", "Enable raw output toggle", "checkbox", false,
     "Show toggle button to display messages as plain text instead of Markdown-formatted content"},
    {"jsSandboxEnabled", "JavaScript sandbox tool", "checkbox", false,
     "Expose a run_javascript tool to the model. Code runs in a Web Worker inside a sandboxed iframe with an opaque origin, isolated from the WebUI and its API, with a hard timeout."},
    {"symbolicMathEnabled", "Symbolic math (nerdamer)", "checkbox", false,
     "Enable the nerdamer symbolic math tool for the model."},
    {"customJson", "Custom JSON", "textarea", false,
     "Custom JSON parameters to send to the API. Must be valid JSON format."},
    {"customCss", "Custom CSS", "textarea", false,
     "Custom CSS to inject into the interface."},
    {"logLevel", "Log level", "loglevel"},
};
} // namespace

SettingsDialog::SettingsDialog(QJsonObject cfg, const QJsonObject &props,
                               const Theme::Tokens &tokens, QWidget *parent)
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
    layout->setSpacing(16); // v2: gap-4 after the w-64 nav -> content at x360

    // v2 SettingsChatDesktopSidebar: w-64 nav at x88 (after the app strip),
    // icon + label, active accent state, version footer
    auto *nav = new QWidget(this);
    nav->setObjectName(QStringLiteral("settingsNav"));
    nav->setFixedWidth(256);
    nav->setContentsMargins(0, 0, 0, 0);
    auto *navPad = new QWidget(this);
    navPad->setFixedWidth(72); // app strip room; +16 gap-4 -> nav lands x88
    navPad->setAutoFillBackground(false);
    auto *stripL = new QVBoxLayout(navPad);
    stripL->setContentsMargins(16, 16, 16, 0); // v2: aside ml-2 + px-2 (strip x16-52)
    stripL->setSpacing(0);
    auto stripBtn = [this, navPad](const QString &icon, const QString &tip) {
        auto *b = new QPushButton(navPad);
        b->setProperty("strip", true);
        b->setFixedSize(36, 36); // v2: strip buttons h-9 w-9
        b->setIconSize(QSize(16, 16)); // v2: size-4 icons
        b->setIcon(Icons::get(icon, Icons::color()));
        b->setToolTip(tip);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };
    auto *stripLogo = new QPushButton(navPad);
    stripLogo->setProperty("strip", true);
    stripLogo->setProperty("logoBtn", true);
    stripLogo->setFixedSize(36, 36); // v2: logo tile h-9 w-9
    stripLogo->setIconSize(QSize(16, 16));
    stripLogo->setIcon(Icons::logo(Icons::color()));
    stripLogo->setToolTip(I18n::t(QStringLiteral("Back to Chat")));
    connect(stripLogo, &QPushButton::clicked, this, &QDialog::close);
    stripL->addWidget(stripLogo, 0, Qt::AlignLeft);
    stripL->addSpacing(8); // v2: logo row + mt-2 -> actions at y60
    auto *bNew = stripBtn(QStringLiteral("square-pen"), I18n::t(QStringLiteral("New chat")));
    connect(bNew, &QPushButton::clicked, this, &QDialog::close);
    stripL->addWidget(bNew, 0, Qt::AlignLeft);
    stripL->addSpacing(4); // v2: gap-1 between strip buttons
    auto *bSearch = stripBtn(QStringLiteral("search"), I18n::t(QStringLiteral("Search")));
    connect(bSearch, &QPushButton::clicked, this, &QDialog::close);
    stripL->addWidget(bSearch, 0, Qt::AlignLeft);
    stripL->addSpacing(4);
    auto *bMcp = stripBtn(QStringLiteral("boxes"), I18n::t(QStringLiteral("MCP Servers")));
    connect(bMcp, &QPushButton::clicked, this, [this]() { accept(); });
    stripL->addWidget(bMcp, 0, Qt::AlignLeft);
    stripL->addSpacing(4);
    auto *bSettings = stripBtn(QStringLiteral("settings"), I18n::t(QStringLiteral("Settings")));
    bSettings->setProperty("mcpActive", true); // active strip tile (v2 bg-muted)
    stripL->addWidget(bSettings, 0, Qt::AlignLeft);
    stripL->addStretch(1);
    auto *nl = new QVBoxLayout(nav);
    nl->setContentsMargins(0, 16, 0, 16); // v2: container py-4
    nl->setSpacing(4);

    auto *headingRow = new QHBoxLayout;
    headingRow->setContentsMargins(0, 8, 0, 8); // v2: row py-2
    headingRow->setSpacing(8);                  // v2: gap-2
    auto *headingIcon = new QLabel(nav);
    headingIcon->setPixmap(Icons::get(QStringLiteral("settings"), Icons::color()).pixmap(24, 24)); // v2: h-6 md:h-6
    headingRow->addWidget(headingIcon);
    auto *heading = new QLabel(I18n::t(QStringLiteral("Settings")), nav);
    heading->setObjectName(QStringLiteral("settingsHeading"));
    QFont hf = heading->font();
    hf.setPixelSize(24);
    hf.setWeight(QFont::DemiBold);
    heading->setFont(hf);
    headingRow->addWidget(heading);
    headingRow->addStretch(1);
    nl->addLayout(headingRow);
    nl->addSpacing(8);

    m_navList = new QListWidget(nav);
    m_navList->setObjectName(QStringLiteral("settingsNavList"));
    m_navList->setFrameShape(QFrame::NoFrame);
    m_navList->setIconSize(QSize(16, 16)); // v2: h-5 w-5 icons
    m_navList->setItemDelegate(new NavDelegate(tokens.accent, tokens.foreground,
                                               tokens.mutedForeground, m_navList));
    m_navList->setFocusPolicy(Qt::NoFocus);
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
        item->setSizeHint(QSize(0, 40)); // v2: 36px item + space-y-1 = 40px pitch
        m_navList->addItem(item);
    }
    nl->addWidget(m_navList, 1);

    // version footer (v2: "Llama UI Native vX (build Y)", 11px, 50% opacity)
    auto *ver = new QLabel(QStringLiteral("Llama UI Native v0.5.0 (build 0x07D21)"), nav);
    ver->setObjectName(QStringLiteral("settingsVersion"));
    ver->setAlignment(Qt::AlignHCenter);
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
    m_stack->addWidget(buildTools());
    m_stack->addWidget(buildImportExport());

    auto *rightCol = new QVBoxLayout;
    rightCol->setContentsMargins(0, 0, 0, 0);
    rightCol->setSpacing(0);
    rightCol->addWidget(m_stack, 1);
    layout->addLayout(rightCol, 1);

    // footer: reset-to-default + save (v2 SettingsFooter: sticky bottom,
    // max-w-3xl box at x496-1264, p-6, justify-between — content scrolls
    // UNDER it, so the footer overlays the stack's bottom strip)
    auto *buttons = new QWidget(this);
    buttons->setObjectName(QStringLiteral("settingsFooter"));
    buttons->setAttribute(Qt::WA_StyledBackground);
    buttons->setFixedWidth(768); // v2: max-w-3xl
    buttons->setMinimumHeight(84); // v2: p-6 (48) + buttons (36)
    buttons->setMaximumHeight(84);
    auto *bl = new QHBoxLayout(buttons);
    bl->setContentsMargins(24, 24, 24, 24); // v2: p-6
    bl->setSpacing(8);
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
    bl->addWidget(reset);
    bl->addStretch(1);
    auto *save = new QPushButton(I18n::t(QStringLiteral("Save settings")));
    connect(save, &QPushButton::clicked, this, &SettingsDialog::apply);
    bl->addWidget(save);

    // overlay: footer sits on top of the stack (sticky bottom-0), so the
    // last field rows scroll under it like the v2
    buttons->setParent(m_stack);
    buttons->move(136, 816); // stack x360 + 136 = x496, y900-84 = 816
    buttons->raise();
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

    auto *center = new QWidget(host);
    center->setObjectName(QStringLiteral("settingsCenter"));
    center->setMaximumWidth(768); // v2: max-w-3xl at x496-1264
    center->setMinimumWidth(768);
    // v2: max-w-3xl centered in the 1040px content column, ignoring the
    // vertical scrollbar (viewport 1032) -> x496 -> cards at x520
    auto *crow = new QHBoxLayout;
    crow->setContentsMargins(0, 0, 0, 0);
    crow->setSpacing(0);
    crow->addSpacing(136);
    crow->addWidget(center);
    crow->addStretch(1);
    col->addLayout(crow);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(24, 112, 24, 0); // v2: p-6 + md:pt-28 -> cards x520
    cl->setSpacing(24);                    // v2: space-y-6 between cards

    // v2 section header: icon + title + bottom border (pb-6)
    auto *headerRow = new QWidget(center);
    headerRow->setObjectName(QStringLiteral("settingsSectionHeader"));
    headerRow->setAttribute(Qt::WA_StyledBackground);
    auto *hrl = new QHBoxLayout(headerRow);
    hrl->setContentsMargins(0, 0, 0, 24); // v2: pb-6 -> border at y165
    hrl->setSpacing(8);
    auto *secIcon = new QLabel(headerRow);
    secIcon->setPixmap(Icons::get(QStringLiteral("sliders-horizontal"), Icons::color())
                           .pixmap(18, 18));
    hrl->addWidget(secIcon);
    auto *heading = new QLabel(title, headerRow);
    heading->setObjectName(QStringLiteral("settingsSectionTitle"));
    heading->setMinimumHeight(28); // v2: title box h=28
    hrl->addWidget(heading);
    hrl->addStretch(1);
    cl->addWidget(headerRow);

    auto addCard = [&](const QStringList &labels, const QList<QWidget *> &controls,
                       const QStringList &helps = {}) {
        auto makeHelp = [](const QString &text, QWidget *parent) {
            auto *help = new QLabel(text, parent);
            help->setObjectName(QStringLiteral("settingsHelp"));
            help->setWordWrap(true);
            // v2: text-sm lh-4 -> exactly 16px per wrapped line
            const QFontMetrics fm(help->font());
            const int lines = qMax(1, (int)std::ceil(
                fm.horizontalAdvance(text) / 690.0));
            help->setFixedHeight(16 * lines);
            help->setMaximumHeight(16 * lines);
            return help;
        };
        auto *card = new QWidget(center);
        card->setObjectName(QStringLiteral("settingsCard"));
        card->setAttribute(Qt::WA_StyledBackground);
        auto *cardL = new QVBoxLayout(card);
        cardL->setContentsMargins(0, 0, 0, 0);
        // v2: input cards use space-y-2 (8px); checkbox cards 4px
        const bool checkCard = std::any_of(
            controls.begin(), controls.end(),
            [](QWidget *w) { return qobject_cast<QCheckBox *>(w) != nullptr; });
        cardL->setSpacing(checkCard ? 4 : 8);
        for (int i = 0; i < controls.size(); ++i) {
            auto *lbl = new QLabel(labels.at(i), card);
            lbl->setObjectName(QStringLiteral("settingsFieldLabel"));
            const bool isSelect = qobject_cast<QComboBox *>(controls.at(i)) != nullptr;
            const bool isCheck = qobject_cast<QCheckBox *>(controls.at(i)) != nullptr;
            if (isCheck) {
                // v2 checkbox card: [checkbox] label on one row, 4px gap,
                // help below (card h=40)
                auto *crow = new QHBoxLayout;
                crow->setContentsMargins(0, 0, 0, 0);
                crow->setSpacing(12); // v2: space-x-3 -> label at x548
                auto *cbWrap = new QVBoxLayout;
                cbWrap->setContentsMargins(0, 4, 0, 0); // v2: checkbox at +4
                cbWrap->setSpacing(0);
                controls.at(i)->setFixedWidth(16);
                controls.at(i)->setMinimumHeight(16);
                controls.at(i)->setMaximumHeight(16);
                cbWrap->addWidget(controls.at(i));
                crow->addLayout(cbWrap, 0);
                crow->addWidget(lbl, 1, Qt::AlignTop);
                cardL->addLayout(crow);
                if (i < helps.size() && !helps.at(i).isEmpty()) {
                    cardL->addWidget(makeHelp(helps.at(i), card));
                }
                continue;
            }
            // v2 SettingsChatFields: label above, control full-width below
            cardL->addWidget(lbl);
            if (isSelect) {
                controls.at(i)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                cardL->addWidget(controls.at(i));
                if (i < helps.size() && !helps.at(i).isEmpty()) {
                    cardL->addWidget(makeHelp(helps.at(i), card));
                }
                continue;
            }
            // v2 SettingsChatFields: label above, control full-width below
            controls.at(i)->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            controls.at(i)->setMinimumWidth(200);
            cardL->addWidget(controls.at(i));
            if (i < helps.size() && !helps.at(i).isEmpty()) {
                cardL->addWidget(makeHelp(helps.at(i), card));
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
        if (!controls.isEmpty())
            addCard(labels, controls,
                    {QStringLiteral("Choose the color theme for the interface.")});
    }

    // fields render in registry order; each checkbox is its own card
    // (v2: one checkbox + help per card)
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
            flushChecks();
            checkLabels << f.label;
            checkHelps << f.help;
            checkControls << makeControl(f);
            flushChecks();
        } else {
            flushChecks();
            addCard({f.label}, {makeControl(f)}, {f.help});
        }
    }
    flushChecks();

    cl->addStretch(1);
    scroll->setWidget(host);
    return scroll;
}

QWidget *SettingsDialog::buildTools() {
    // v2 SettingsChatToolsTab: disclosure button "Built in tools 10 tools"
    // + a table (Tool | Enabled | Always allow) with 32px rows; collapsed
    // by default; divider + localStorage note at the bottom
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    auto *col = new QVBoxLayout(host);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(12);

    auto *center = new QWidget(host);
    center->setObjectName(QStringLiteral("settingsCenter"));
    center->setMaximumWidth(768);
    center->setMinimumWidth(768);
    auto *crow = new QHBoxLayout;
    crow->setContentsMargins(0, 0, 0, 0);
    crow->setSpacing(0);
    crow->addSpacing(136);
    crow->addWidget(center);
    crow->addStretch(1);
    col->addLayout(crow);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(24, 112, 24, 0);
    cl->setSpacing(24);

    auto *headerRow = new QWidget(center);
    headerRow->setObjectName(QStringLiteral("settingsSectionHeader"));
    headerRow->setAttribute(Qt::WA_StyledBackground);
    auto *hrl = new QHBoxLayout(headerRow);
    hrl->setContentsMargins(0, 0, 0, 24);
    hrl->setSpacing(8);
    auto *secIcon = new QLabel(headerRow);
    secIcon->setPixmap(Icons::get(QStringLiteral("wrench"), Icons::color()).pixmap(18, 18));
    hrl->addWidget(secIcon);
    auto *heading = new QLabel(I18n::t(QStringLiteral("Tools")), headerRow);
    heading->setObjectName(QStringLiteral("settingsSectionTitle"));
    heading->setMinimumHeight(28);
    hrl->addWidget(heading);
    hrl->addStretch(1);
    cl->addWidget(headerRow);

    // disclosure button (v2: flex w-full rounded-lg px-3 py-2 text-sm)
    auto *toggle = new QPushButton(
        QStringLiteral("  %1  %2").arg(I18n::t(QStringLiteral("Built in tools")),
                                       QStringLiteral("10 tools")),
        center);
    toggle->setObjectName(QStringLiteral("toolsDisclosure"));
    toggle->setProperty("outline", true);
    toggle->setCursor(Qt::PointingHandCursor);
    cl->addWidget(toggle);

    // table container (v2: ml-4 border-l pl-2), hidden until expanded
    auto *table = new QWidget(center);
    table->setObjectName(QStringLiteral("toolsTable"));
    auto *tl = new QVBoxLayout(table);
    tl->setContentsMargins(0, 0, 0, 0);
    tl->setSpacing(0);

    struct ToolRow {
        QString key, label;
    };
    const QList<ToolRow> tools{
        {QStringLiteral("toolCalculateEnabled"), QStringLiteral("calculate")},
        {QStringLiteral("toolFetchUrlEnabled"), QStringLiteral("fetch_url")},
        {QStringLiteral("toolToTableEnabled"), QStringLiteral("to_table")},
        {QStringLiteral("toolJsonEnabled"), QStringLiteral("json_tool")},
        {QStringLiteral("toolClipboardEnabled"), QStringLiteral("clipboard")},
        {QStringLiteral("toolNotifyEnabled"), QStringLiteral("notify")},
        {QStringLiteral("toolTodoEnabled"), QStringLiteral("todo_list")},
        {QStringLiteral("toolWeatherEnabled"), QStringLiteral("weather")},
        {QStringLiteral("toolWikipediaEnabled"), QStringLiteral("wikipedia")},
        {QStringLiteral("toolPlotChartEnabled"), QStringLiteral("plot_chart")},
    };
    auto *headRow = new QHBoxLayout;
    headRow->setContentsMargins(0, 0, 0, 0);
    headRow->setSpacing(0);
    auto *headTool = new QLabel(QStringLiteral("Tool"), table);
    headTool->setObjectName(QStringLiteral("toolsColHead"));
    headRow->addWidget(headTool, 1);
    auto *headOn = new QLabel(QStringLiteral("Enabled"), table);
    headOn->setObjectName(QStringLiteral("toolsColHead"));
    headOn->setFixedWidth(64);
    headRow->addWidget(headOn, 0, Qt::AlignHCenter);
    auto *headAllow = new QLabel(QStringLiteral("Always allow"), table);
    headAllow->setObjectName(QStringLiteral("toolsColHead"));
    headAllow->setFixedWidth(80);
    headRow->addWidget(headAllow, 0, Qt::AlignHCenter);
    tl->addLayout(headRow);

    for (const auto &t : tools) {
        auto *row = new QHBoxLayout;
        row->setContentsMargins(0, 0, 0, 0);
        row->setSpacing(0);
        auto *name = new QLabel(t.label, table);
        name->setObjectName(QStringLiteral("toolsRowName"));
        row->addWidget(name, 1);
        auto *on = new QCheckBox(table);
        on->setChecked(m_cfg.value(t.key).toBool(true));
        on->setFixedWidth(64);
        connect(on, &QCheckBox::toggled, this, [this, t](bool v) {
            m_cfg.insert(t.key, v);
        });
        row->addWidget(on, 0, Qt::AlignHCenter);
        auto *allow = new QCheckBox(table);
        allow->setChecked(m_cfg.value(t.key + QStringLiteral("Always")).toBool(false));
        allow->setFixedWidth(80);
        connect(allow, &QCheckBox::toggled, this, [this, t](bool v) {
            m_cfg.insert(t.key + QStringLiteral("Always"), v);
        });
        row->addWidget(allow, 0, Qt::AlignHCenter);
        tl->addLayout(row);
    }
    cl->addWidget(table);
    table->setVisible(false);

    connect(toggle, &QPushButton::clicked, this, [table, toggle]() {
        const bool show = !table->isVisible();
        table->setVisible(show);
        toggle->setText(QStringLiteral("  %1  %2")
                            .arg(I18n::t(QStringLiteral("Built in tools")),
                                 QStringLiteral("10 tools")));
    });

    // divider + note (v2: mt-8 border-t pt-6 + text-xs muted)
    auto *divider = new QWidget(center);
    divider->setObjectName(QStringLiteral("toolsDivider"));
    divider->setFixedHeight(1);
    divider->setAttribute(Qt::WA_StyledBackground);
    cl->addWidget(divider);
    auto *note = new QLabel(
        I18n::t(QStringLiteral("Settings are saved in browser's localStorage")), center);
    note->setObjectName(QStringLiteral("settingsHelp"));
    cl->addWidget(note);
    cl->addStretch(1);

    scroll->setWidget(host);
    return scroll;
}

QWidget *SettingsDialog::buildImportExport() {
    // v2 SettingsChatImportExportTab: section header, then plain group
    // titles (16px) with desc (14px) + outline button below each
    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *host = new QWidget;
    auto *col = new QVBoxLayout(host);
    col->setContentsMargins(0, 0, 0, 0);
    col->setSpacing(12);

    auto *center = new QWidget(host);
    center->setObjectName(QStringLiteral("settingsCenter"));
    center->setMaximumWidth(768);
    center->setMinimumWidth(768);
    auto *crow = new QHBoxLayout;
    crow->setContentsMargins(0, 0, 0, 0);
    crow->setSpacing(0);
    crow->addSpacing(136);
    crow->addWidget(center);
    crow->addStretch(1);
    col->addLayout(crow);
    auto *cl = new QVBoxLayout(center);
    cl->setContentsMargins(24, 112, 24, 0);
    cl->setSpacing(0); // v2 gaps are explicit below

    // section header (icon + title + bottom border)
    auto *headerRow = new QWidget(center);
    headerRow->setObjectName(QStringLiteral("settingsSectionHeader"));
    headerRow->setAttribute(Qt::WA_StyledBackground);
    auto *hrl = new QHBoxLayout(headerRow);
    hrl->setContentsMargins(0, 0, 0, 24);
    hrl->setSpacing(8);
    auto *secIcon = new QLabel(headerRow);
    secIcon->setPixmap(Icons::get(QStringLiteral("download"), Icons::color()).pixmap(18, 18));
    hrl->addWidget(secIcon);
    auto *heading = new QLabel(I18n::t(QStringLiteral("Import/Export")), headerRow);
    heading->setObjectName(QStringLiteral("settingsSectionTitle"));
    heading->setMinimumHeight(28);
    hrl->addWidget(heading);
    hrl->addStretch(1);
    cl->addWidget(headerRow);
    cl->addSpacing(24); // v2: mb-6 -> first group title at y189

    auto addGroup = [&](const QString &title, const QString &desc,
                        const QString &buttonText, std::function<void()> fn,
                        bool destructive = false) {
        if (!title.isEmpty()) {
            auto *g = new QLabel(title, center);
            g->setObjectName(QStringLiteral("settingsGroupTitle"));
            g->setMinimumHeight(24); // v2: h3 line box
            g->setFixedHeight(24);
            cl->addWidget(g);
            cl->addSpacing(56); // v2: title y189 -> desc y269
        }
        auto *d = new QLabel(desc, center);
        d->setObjectName(QStringLiteral("settingsDesc"));
        d->setWordWrap(true);
        d->setFixedHeight(40); // v2: two-line desc
        cl->addWidget(d);
        cl->addSpacing(20); // v2: desc y269 -> button y329
        auto *btn = new QPushButton(buttonText, center);
        btn->setProperty("outline", true);
        if (destructive) btn->setProperty("destructive", true);
        connect(btn, &QPushButton::clicked, this, fn);
        cl->addWidget(btn, 0, Qt::AlignLeft);
        cl->addSpacing(64); // v2: card pitch 160 (desc 40 + gap 20 + btn 36 + 64)
    };

    addGroup(I18n::t(QStringLiteral("Conversations")),
             I18n::t(QStringLiteral("Download your conversations as a ZIP of JSONL files. This includes all messages, attachments, and conversation history.")),
             I18n::t(QStringLiteral("Export conversations")),
             [this]() {
                 QFileDialog::getSaveFileName(this, I18n::t(QStringLiteral("Export conversations")),
                                              QStringLiteral("conversations.json"));
             });
    addGroup(QString(),
             I18n::t(QStringLiteral("Import one or more conversations from a previously exported ZIP or JSONL file. This will merge with your existing conversations.")),
             I18n::t(QStringLiteral("Import conversations")),
             []() {});
    addGroup(QString(),
             I18n::t(QStringLiteral("Permanently delete all conversations and their messages. This action cannot be undone. Consider exporting your conversations first if you want to keep a backup.")),
             I18n::t(QStringLiteral("Delete all conversations")),
             [this]() {
                 const auto ret = QMessageBox::question(
                     this, I18n::t(QStringLiteral("Delete all conversations")),
                     I18n::t(QStringLiteral("Are you sure?")));
                 if (ret == QMessageBox::Yes) emit clearAllConversationsRequested();
             }, true);

    addGroup(I18n::t(QStringLiteral("Settings")),
             I18n::t(QStringLiteral("Export your chat settings and preferences as a JSON file.")),
             I18n::t(QStringLiteral("Export settings")),
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

    cl->addStretch(1);
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
