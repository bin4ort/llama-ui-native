#include "MessageView.h"
#include "../Config.h"
#include "../Theme.h"
#include "../i18n.h"
#include <QAbstractTextDocumentLayout>
#include <QDesktopServices>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QRegularExpression>
#include <functional>

namespace {

/** Minimal markdown -> HTML (UI-SPEC §07). Theme-aware colors: links use
 *  the primary token, table headers use muted/50. */
QString mdToHtml(const QString &md, const QColor &linkColor, const QColor &thBg) {
    QString html = md.toHtmlEscaped();
    // fenced code blocks
    QRegularExpression fence(R"(```(\w*)\n([\s\S]*?)```)");
    QString out;
    int pos = 0;
    for (auto it = fence.globalMatch(html); it.hasNext();) {
        auto m = it.next();
        out += html.mid(pos, m.capturedStart() - pos);
        out += QStringLiteral("<pre style='background-color:rgba(128,128,128,0.15);"
                              "padding:8px;border-radius:8px;font-family:monospace'>") +
               m.captured(2).replace("\n", "<br>") + QStringLiteral("</pre>");
        pos = m.capturedEnd();
    }
    out += html.mid(pos);
    html = out;
    // headings
    for (int h = 1; h <= 3; ++h)
        html.replace(QRegularExpression(QStringLiteral("^#{%1} ([^\n]+)$").arg(h),
                                        QRegularExpression::MultilineOption),
                     QStringLiteral("<h%1>\1</h%1>").arg(h));
    // bold / italic / strikethrough / inline code / links / images
    html.replace(QRegularExpression(R"(\*\*([^*]+)\*\*)"), QStringLiteral("<b>\1</b>"));
    html.replace(QRegularExpression(R"(\*([^*\n]+)\*)"), QStringLiteral("<i>\1</i>"));
    html.replace(QRegularExpression(R"(~~([^~]+)~~)"), QStringLiteral("<s>\1</s>"));
    html.replace(QRegularExpression(R"(`([^`]+)`)"),
                 QStringLiteral("<code style='background-color:rgba(128,128,128,0.15);"
                                "border-radius:4px;padding:1px 4px;font-family:monospace'>\1</code>"));
    html.replace(QRegularExpression(R"(\[([^\]]+)\]\(([^)\s]+)\))"),
                 QStringLiteral("<a href='\2' style='color:%1'>\1</a>").arg(linkColor.name()));
    html.replace(QRegularExpression(R"(!\[([^\]]*)\]\(([^)\s]+)\))"),
                 QStringLiteral("<img src='\2' width='60%' alt='\1'>"));
    // task lists: - [ ] / - [x]
    const QRegularExpression::PatternOptions ml = QRegularExpression::MultilineOption;
    html.replace(QRegularExpression(R"(^- \[x\] (.*)$)", ml),
                 QStringLiteral("☑ \1"));
    html.replace(QRegularExpression(R"(^- \[ \] (.*)$)", ml),
                 QStringLiteral("☐ \1"));
    // blockquotes
    html.replace(QRegularExpression(QStringLiteral("^&gt; ([^\n]+)$"), ml),
                 QStringLiteral("<blockquote>\1</blockquote>"));
    // tables + lists (UI-SPEC §07) — one scoped line-based pass so the
    // list wrapper never wraps a table (invalid HTML gets dropped by QLabel)
    {
        const QStringList lines = html.split('\n');
        QString out;
        int i = 0;
        while (i < lines.size()) {
            const QString line = lines.at(i).trimmed();
            if (line.startsWith('|') && i + 1 < lines.size()) {
                QStringList rows;
                while (i < lines.size() && lines.at(i).trimmed().startsWith('|')) {
                    rows.append(lines.at(i).trimmed());
                    ++i;
                }
                if (rows.size() >= 2) {
                    auto cellsOf = [](const QString &r) {
                        return r.split('|', Qt::SkipEmptyParts);
                    };
                    const QStringList header = cellsOf(rows.first());
                    int bodyStart = 1;
                    if (rows.size() >= 2) {
                        bool sep = true;
                        for (const auto &c : cellsOf(rows.at(1))) {
                            const QString t = c.trimmed();
                            if (t.isEmpty()) continue;
                            for (QChar ch : t)
                                if (ch != '-') sep = false;
                        }
                        if (sep) bodyStart = 2;
                    }
                    // QTextDocument supports native table attributes
                    // (style= is ignored): border/cellpadding/bgcolor.
                    QString tbl = QStringLiteral("<table border='1' cellpadding='4' cellspacing='0'>");
                    tbl += QStringLiteral("<tr>");
                    for (const auto &h : header)
                        tbl += QStringLiteral("<th bgcolor='%1'>").arg(thBg.name()) +
                               h.trimmed() + QStringLiteral("</th>");
                    tbl += QStringLiteral("</tr>");
                    for (int r = bodyStart; r < rows.size(); ++r) {
                        tbl += QStringLiteral("<tr>");
                        for (const auto &c : cellsOf(rows.at(r)))
                            tbl += QStringLiteral("<td>") + c.trimmed() + QStringLiteral("</td>");
                        tbl += QStringLiteral("</tr>");
                    }
                    tbl += QStringLiteral("</table>");
                    out += tbl;
                    continue;
                }
                out += rows.join('\n') + '\n';
            } else if (line.startsWith('-') || line.startsWith('*') ||
                       QRegularExpression(QStringLiteral("^\\d+\\. ")).match(line).hasMatch()) {
                const bool ordered = QRegularExpression(QStringLiteral("^\\d+\\. ")).match(line).hasMatch();
                QStringList items;
                while (i < lines.size()) {
                    const QString l = lines.at(i).trimmed();
                    const auto m = QRegularExpression(
                        QStringLiteral("^(?:[-*]|\\d+\\.) (.*)$")).match(l);
                    if (!m.hasMatch()) break;
                    items.append(m.captured(1));
                    ++i;
                }
                if (items.size() >= 1) {
                    out += ordered ? QStringLiteral("<ol>") : QStringLiteral("<ul>");
                    for (const auto &it : items)
                        out += QStringLiteral("<li>") + it + QStringLiteral("</li>");
                    out += ordered ? QStringLiteral("</ol>") : QStringLiteral("</ul>");
                    continue;
                }
            }
            out += line + '\n';
            ++i;
        }
        html = out;
    }
    // paragraphs / line breaks
    html.replace(QRegularExpression(QStringLiteral("\n\n")), QStringLiteral("<br><br>"));
    html.replace(QRegularExpression(QStringLiteral("\n")), QStringLiteral("<br>"));
    return html;
}

/** One chat message: label + hover-revealed action row (UI-SPEC §3.9). */
class MessageRow : public QWidget {
public:
    MessageRow(QWidget *parent) : QWidget(parent) {
        setAttribute(Qt::WA_Hover);
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(2);
    }
    void addLabel(QLabel *l) { m_label = l; m_layout->addWidget(l); }
    void addActions(QWidget *actions, Qt::Alignment align) {
        m_actions = actions;
        m_actions->setVisible(false);
        m_layout->addWidget(actions, 0, align);
    }
    void setActionsVisible(bool v) {
        if (m_actions) m_actions->setVisible(v && m_label && !m_label->text().isEmpty());
    }
    bool event(QEvent *ev) override {
        if (ev->type() == QEvent::HoverEnter) setActionsVisible(true);
        else if (ev->type() == QEvent::HoverLeave) setActionsVisible(false);
        return QWidget::event(ev);
    }
private:
    QVBoxLayout *m_layout;
    QLabel *m_label = nullptr;
    QWidget *m_actions = nullptr;
};

} // namespace

MessageView::MessageView(QWidget *parent) : QScrollArea(parent) {
    setObjectName(QStringLiteral("messages"));
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_container = new QWidget;
    m_layout = new QVBoxLayout(m_container);
    m_layout->setContentsMargins(24, 24, 24, 24);
    m_layout->setSpacing(12);
    setWidget(m_container);
    showGreeting();
}

void MessageView::clearLayout() {
    while (m_layout->count()) {
        auto *item = m_layout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_streamingLabel = nullptr;
    m_streamingReasoning = nullptr;
    m_reasoningToggle = nullptr;
    m_streamRaw.clear();
    m_streamReasoning.clear();
}

void MessageView::showGreeting() {
    clearLayout();
    m_layout->addStretch(1);
    auto *title = new QLabel(I18n::t(QStringLiteral("What can I help you with?")));
    title->setObjectName(QStringLiteral("greetingTitle"));
    title->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(title);
    auto *sub = new QLabel(I18n::t(QStringLiteral(
        "Start a conversation with your local AI — ask anything, upload files, or attach tools")));
    sub->setObjectName(QStringLiteral("greetingSub"));
    sub->setAlignment(Qt::AlignCenter);
    sub->setWordWrap(true);
    m_layout->addWidget(sub);
    m_layout->addSpacing(16);
    // suggestion chips (UI-SPEC §6.2)
    const QStringList suggestions{
        I18n::t(QStringLiteral("Write a poem about the ocean")),
        I18n::t(QStringLiteral("Summarize the key ideas of a topic")),
        I18n::t(QStringLiteral("Explain a concept simply")),
        I18n::t(QStringLiteral("Help me write a function")),
    };
    auto *grid = new QWidget(this);
    auto *gl = new QVBoxLayout(grid);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setSpacing(8);
    for (int r = 0; r < 2; ++r) {
        auto *rowW = new QWidget(grid);
        auto *hl = new QHBoxLayout(rowW);
        hl->setContentsMargins(0, 0, 0, 0);
        hl->setSpacing(8);
        hl->addStretch(1);
        for (int c = 0; c < 2; ++c) {
            const int idx = r * 2 + c;
            if (idx >= suggestions.size()) break;
            auto *chip = new QPushButton(QStringLiteral("✦  ") + suggestions.at(idx));
            chip->setProperty("chip", true);
            connect(chip, &QPushButton::clicked, this,
                    [this, s = suggestions.at(idx)]() { emit suggestionClicked(s); });
            hl->addWidget(chip);
        }
        hl->addStretch(1);
        gl->addWidget(rowW);
    }
    m_layout->addWidget(grid);
    m_layout->addStretch(1);
}

void MessageView::addMessage(const ChatMessage &msg) {
    // drop greeting once real content arrives
    if (m_layout->count() > 0) {
        auto *w0 = m_layout->itemAt(0)->widget();
        if (!w0 || w0->objectName() != QStringLiteral("chatRow")) clearLayout();
    }

    const QString palette = Config::value(QStringLiteral("theme")).toString();
    const QColor link = Theme::tokens(palette).primary;
    const QColor thBg = Theme::tokens(palette).muted;

    auto *row = new MessageRow(this);
    row->setObjectName(QStringLiteral("chatRow"));
    QLabel *label = nullptr;
    if (msg.role == ChatMessage::User) {
        const bool md = Config::value(QStringLiteral("renderUserContentAsMarkdown")).toBool();
        label = new QLabel(md ? mdToHtml(msg.content, link, thBg) : msg.content.toHtmlEscaped());
        label->setObjectName(QStringLiteral("userBubble"));
        label->setTextFormat(md ? Qt::RichText : Qt::PlainText);
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        row->addLabel(label);
        m_layout->addWidget(row, 0, Qt::AlignRight);
    } else if (msg.role == ChatMessage::System) {
        if (msg.type == QStringLiteral("persona")) {
            auto *pill = new QLabel(I18n::t(QStringLiteral("System message")));
            pill->setObjectName(QStringLiteral("systemPill"));
            pill->setAlignment(Qt::AlignCenter);
            m_layout->addWidget(pill);
        }
        label = new QLabel(mdToHtml(msg.content, link, thBg));
        label->setObjectName(QStringLiteral("systemBubble"));
        label->setTextFormat(Qt::RichText);
        label->setWordWrap(true);
        row->addLabel(label);
        m_layout->addWidget(row, 0, Qt::AlignRight);
    } else {
        label = new QLabel(mdToHtml(msg.content, link, thBg));
        label->setObjectName(QStringLiteral("assistantText"));
        label->setWordWrap(true);
        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(false);
        row->addLabel(label);
        m_layout->addWidget(row);
    }

    // hover action row (UI-SPEC §3.9): copy / regenerate / edit / delete
    auto *actions = new QWidget(row);
    auto *al = new QHBoxLayout(actions);
    al->setContentsMargins(0, 0, 0, 0);
    al->setSpacing(2);
    auto makeAction = [&](const QString &icon, const QString &tip,
                          std::function<void()> fn) {
        auto *b = new QPushButton(icon, actions);
        b->setProperty("ghost", true);
        b->setToolTip(tip);
        connect(b, &QPushButton::clicked, this, std::move(fn));
        al->addWidget(b);
    };
    makeAction(QStringLiteral("⧉"), I18n::t(QStringLiteral("Copy")),
               [this, id = msg.id, content = msg.content]() {
                   emit copyRequested(id, content);
               });
    if (msg.role != ChatMessage::System) {
        makeAction(QStringLiteral("↻"), I18n::t(QStringLiteral("Regenerate")),
                   [this, id = msg.id]() { emit regenerateRequested(id); });
        if (msg.role == ChatMessage::User)
            makeAction(QStringLiteral("✎"), I18n::t(QStringLiteral("Edit")),
                       [this, id = msg.id]() { emit editRequested(id); });
        makeAction(QStringLiteral("✕"), I18n::t(QStringLiteral("Delete")),
                   [this, id = msg.id]() { emit deleteRequested(id); });
    }
    al->addStretch(1);
    row->addActions(actions, msg.role == ChatMessage::User ? Qt::AlignRight : Qt::AlignLeft);

    if (msg.streaming) {
        m_streamingLabel = label;
        m_streamingReasoning = nullptr;
        m_streamId = msg.id;
        m_streamRaw.clear();
        m_streamReasoning.clear();
        label->setText(QStringLiteral("…"));
    }
    // keep the view pinned to the bottom
    QTimer::singleShot(0, this, [this]() {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    });
}

void MessageView::updateStreaming(const QString &delta) {
    if (!m_streamingLabel) return;
    m_streamRaw += delta;
    const QString palette = Config::value(QStringLiteral("theme")).toString();
    const QColor link = Theme::tokens(palette).primary;
    const QColor thBg = Theme::tokens(palette).muted;
    m_streamingLabel->setText(mdToHtml(m_streamRaw, link, thBg));
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void MessageView::updateReasoning(const QString &delta) {
    if (!m_streamingLabel) return;
    m_streamReasoning += delta;
    if (!m_streamingReasoning) {
        // collapsible "Thought" block above the answer (UI-SPEC §3.6)
        auto *host = new QWidget(this);
        host->setObjectName(QStringLiteral("reasoningHost"));
        auto *vl = new QVBoxLayout(host);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(2);
        m_reasoningToggle = new QPushButton(QStringLiteral("▾ ") + I18n::t(QStringLiteral("Thought")),
                                            host);
        m_reasoningToggle->setProperty("reasoningToggle", true);
        connect(m_reasoningToggle, &QPushButton::clicked, this, [this]() {
            m_streamReasoningVisible = !m_streamReasoningVisible;
            if (m_streamingReasoning)
                m_streamingReasoning->setVisible(m_streamReasoningVisible);
            m_reasoningToggle->setText(QString(m_streamReasoningVisible ? "▾ " : "▸ ") +
                                       I18n::t(QStringLiteral("Thought")));
        });
        vl->addWidget(m_reasoningToggle);
        m_streamingReasoning = new QLabel(m_streamReasoning.toHtmlEscaped());
        m_streamingReasoning->setObjectName(QStringLiteral("reasoningText"));
        m_streamingReasoning->setWordWrap(true);
        m_streamingReasoning->setTextInteractionFlags(Qt::TextSelectableByMouse);
        vl->addWidget(m_streamingReasoning);
        // insert above the streaming label: find the row containing it
        const int n = m_layout->count();
        for (int i = 0; i < n; ++i) {
            auto *item = m_layout->itemAt(i);
            auto *w = item->widget();
            if (w && w->objectName() == QStringLiteral("chatRow")) {
                m_layout->insertWidget(i, host);
                break;
            }
        }
    } else {
        m_streamingReasoning->setText(m_streamReasoning.toHtmlEscaped());
    }
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

QString MessageView::lastAssistantHtml() const {
    return m_streamingLabel ? m_streamingLabel->text() : QString();
}

void MessageView::clear() {
    clearLayout();
    showGreeting();
}
