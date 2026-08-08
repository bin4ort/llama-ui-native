#include "MessageView.h"
#include "../Config.h"
#include "../Theme.h"
#include "../i18n.h"
#include <QAbstractTextDocumentLayout>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QClipboard>
#include <cstdio>
#include <QFont>
#include <QFontMetrics>
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
 *  the primary token, table headers use muted/50. Fenced code blocks get a
 *  language label + copy link (v2 SyntaxHighlightedCode header). */
QString mdToHtml(const QString &md, const QColor &linkColor, const QColor &thBg,
                 QList<QString> *codeBlocks) {
    QString html = md.toHtmlEscaped();
    // fenced code blocks
    QRegularExpression fence(R"(```(\w*)\n([\s\S]*?)```)");
    QString out;
    int pos = 0;
    int codeIdx = 0;
    for (auto it = fence.globalMatch(html); it.hasNext();) {
        auto m = it.next();
        out += html.mid(pos, m.capturedStart() - pos);
        if (codeBlocks) codeBlocks->append(m.captured(2));
        const QString lang = m.captured(1).isEmpty()
                                 ? QStringLiteral("code")
                                 : m.captured(1);
        out += QStringLiteral("<pre style='background-color:rgba(128,128,128,0.15);"
                              "padding:8px;border-radius:8px;font-family:monospace'>") +
               m.captured(2).replace("\n", "<br>") + QStringLiteral("</pre>") +
               QStringLiteral("<a href='copy-code://%1' style='color:%2;font-size:11px'>")
                   .arg(codeIdx++).arg(linkColor.name()) +
               QStringLiteral("⧉ %1 · %2</a><br>")
                   .arg(I18n::t(QStringLiteral("Copy")), lang);
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

QLabel *makeRichLabel(const QString &html, const QString &objName,
                        QWidget *parent, QList<QString> *blocks) {
    auto *l = new QLabel(html, parent);
    l->setObjectName(objName);
    l->setTextFormat(Qt::RichText);
    l->setWordWrap(true);
    l->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    QObject::connect(l, &QLabel::linkActivated, l, [l, blocks](const QString &href) {
        if (href.startsWith(QStringLiteral("copy-code://"))) {
            const int idx = href.mid(12).toInt();
            if (blocks && idx >= 0 && idx < blocks->size())
                QGuiApplication::clipboard()->setText(blocks->at(idx));
        } else if (href.startsWith(QStringLiteral("http"))) {
            QDesktopServices::openUrl(QUrl(href));
        }
    });
    return l;
}

/** One chat message: label + action row (UI-SPEC §3.9, always visible like
 *  v2 — no hover reveal). */
class MessageRow : public QWidget {
public:
    MessageRow(QWidget *parent) : QWidget(parent) {
        m_layout = new QVBoxLayout(this);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(2);
    }
    void addLabel(QLabel *l) { m_label = l; m_layout->addWidget(l); }
    void addActions(QWidget *actions, Qt::Alignment align) {
        m_actions = actions;
        m_actions->setVisible(true);
        m_layout->addWidget(actions, 0, align);
    }
    void addTimestamp() {
        // v2: muted timestamp under the bubble (my-2)
        auto *ts = new QLabel(I18n::t(QStringLiteral("just now")), this);
        ts->setObjectName(QStringLiteral("msgTimestamp"));
        m_layout->addWidget(ts, 0, Qt::AlignLeft);
    }
    void setActionsVisible(bool v) {
        if (m_actions) m_actions->setVisible(v && m_label && !m_label->text().isEmpty());
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
    setFrameShape(QFrame::NoFrame); // v2: no frame lines around the chat

    m_container = new QWidget;
    m_layout = new QVBoxLayout(m_container);
    m_layout->setContentsMargins(16, 16, 16, 16);
    m_layout->setSpacing(12);
    m_rows = m_layout;
    setWidget(m_container);
    showGreeting();
}

/** v2: user rows sit in the right 80% — a 0.6L-wide centered row puts
 *  its right edge exactly at 0.8L. */
void MessageView::updateUserRowWidth(QWidget *row) {
    row->setFixedWidth(qMax(500, int(0.6 * width())));
}

void MessageView::resizeEvent(QResizeEvent *event) {
    QScrollArea::resizeEvent(event);
    for (auto *row : m_userRows) updateUserRowWidth(row);
}

void MessageView::clearLayout() {
    while (m_rows->count()) {
        auto *item = m_rows->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_streamingLabel = nullptr;
    m_streamingReasoning = nullptr;
    m_reasoningToggle = nullptr;
    m_streamRaw.clear();
    m_streamReasoning.clear();
    m_codeBlocks.clear();
    m_userRows.clear();
}

void MessageView::showGreeting() {
    clearLayout();
    // v2: the greeting hugs the floating form (mb-6), not vertically centered
    m_rows->addStretch(1);
    auto *title = new QLabel(I18n::t(QStringLiteral("Hello there")));
    title->setObjectName(QStringLiteral("greetingTitle"));
    title->setAlignment(Qt::AlignCenter);
    m_rows->addWidget(title);
    auto *sub = new QLabel(I18n::t(QStringLiteral("Type a message or upload files to get started")));
    sub->setObjectName(QStringLiteral("greetingSub"));
    sub->setAlignment(Qt::AlignCenter);
    sub->setWordWrap(true);
    m_rows->addWidget(sub);
    auto *bottomGap = new QWidget(this);
    bottomGap->setFixedHeight(8);
    m_rows->addWidget(bottomGap);
}

void MessageView::addMessage(const ChatMessage &msg) {
    // drop greeting once real content arrives
    if (m_rows->count() > 0) {
        auto *w0 = m_rows->itemAt(0)->widget();
        if (!w0 || w0->objectName() != QStringLiteral("chatRow")) clearLayout();
    }

    const QString palette = Config::value(QStringLiteral("theme")).toString();
    const auto tk = Theme::tokens(palette);
    const QColor link = tk.primary;
    // v2 table header: muted-foreground at ~35% over the card (~87 gray)
    const QColor thBg = QColor::fromRgbF(
        tk.mutedForeground.redF() * 0.35 + tk.card.redF() * 0.65,
        tk.mutedForeground.greenF() * 0.35 + tk.card.greenF() * 0.65,
        tk.mutedForeground.blueF() * 0.35 + tk.card.blueF() * 0.65);

    auto *row = new MessageRow(this);
    row->setObjectName(QStringLiteral("chatRow"));
    QLabel *label = nullptr;
    // attachment names shown above the bubble (UI-SPEC §4.2)
    QStringList attNames;
    for (const auto &a : msg.attachments)
        attNames << a.toObject().value(QStringLiteral("name")).toString();
    if (msg.role == ChatMessage::User) {
        const bool md = Config::value(QStringLiteral("renderUserContentAsMarkdown")).toBool();
        QString body = msg.content;
        if (!attNames.isEmpty())
            body = attNames.join(QStringLiteral("\n")) +
                   (msg.content.isEmpty() ? QString() : QStringLiteral("\n\n") + msg.content);
        if (md) {
            label = makeRichLabel(mdToHtml(body, link, thBg, &m_codeBlocks),
                                  QStringLiteral("userBubble"), row, &m_codeBlocks);
        } else {
            label = new QLabel(body.toHtmlEscaped());
            label->setObjectName(QStringLiteral("userBubble"));
            label->setTextFormat(Qt::PlainText);
            label->setWordWrap(true);
            label->setTextInteractionFlags(Qt::TextSelectableByMouse);
            // bubble sizes to the text (v2): short texts stay single-line
            // so the bubble hugs the content instead of wrapping at the
            // widest word
            if (!msg.content.isEmpty()) {
                QFont f = label->font();
                f.setPixelSize(14);
                const int textW = QFontMetrics(f).horizontalAdvance(msg.content);
                if (textW < 600) label->setWordWrap(false);
            }
        }
        row->addLabel(label);
        m_userRows.append(row);
        updateUserRowWidth(row);
        m_rows->addWidget(row, 0, Qt::AlignHCenter);
    } else if (msg.role == ChatMessage::System) {
        if (msg.type == QStringLiteral("persona")) {
            auto *pill = new QLabel(I18n::t(QStringLiteral("System message")));
            pill->setObjectName(QStringLiteral("systemPill"));
            pill->setAlignment(Qt::AlignCenter);
            m_rows->addWidget(pill);
        }
        label = makeRichLabel(mdToHtml(msg.content, link, thBg, &m_codeBlocks),
                              QStringLiteral("systemBubble"), row, &m_codeBlocks);
        row->addLabel(label);
        m_rows->addWidget(row, 0, Qt::AlignRight);
    } else if (msg.type == QStringLiteral("tool-call")) {
        // v2 ToolCallBlock header (tool name + args)
        const auto calls = QJsonDocument::fromJson(msg.content.toUtf8()).array();
        QStringList parts;
        for (const auto &c : calls) {
            const auto o = c.toObject();
            parts << o.value(QStringLiteral("function")).toObject()
                         .value(QStringLiteral("name")).toString();
        }
        label = new QLabel(QStringLiteral("🔧 ") +
                           parts.join(QStringLiteral(", ")));
        label->setObjectName(QStringLiteral("toolCallRow"));
        label->setWordWrap(true);
        row->addLabel(label);
        m_rows->addWidget(row);
    } else if (msg.type == QStringLiteral("tool-result")) {
        // v2 ToolCallBlock output row (compact)
        label = new QLabel(msg.content.toHtmlEscaped());
        label->setObjectName(QStringLiteral("toolResultRow"));
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        row->addLabel(label);
        m_rows->addWidget(row, 0, Qt::AlignLeft);
    } else {
        label = makeRichLabel(mdToHtml(msg.content, link, thBg, &m_codeBlocks),
                              QStringLiteral("assistantText"), row, &m_codeBlocks);
        row->addLabel(label);
        m_rows->addWidget(row);
    }

    // action row (v2 ChatMessageActionIcons order: copy, edit, regenerate,
    // continue, delete) — always visible, aligned like v2
    auto *actions = new QWidget(row);
    actions->setObjectName(QStringLiteral("messageActions"));
    actions->setAttribute(Qt::WA_StyledBackground);
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
    if (!msg.streaming) {
        if (msg.role != ChatMessage::System)
            makeAction(QStringLiteral("✎"), I18n::t(QStringLiteral("Edit")),
                       [this, id = msg.id]() { emit editRequested(id); });
        if (msg.role == ChatMessage::Assistant) {
            makeAction(QStringLiteral("↻"), I18n::t(QStringLiteral("Regenerate")),
                       [this, id = msg.id]() { emit regenerateRequested(id); });
            if (Config::value(QStringLiteral("enableContinueGeneration")).toBool())
                makeAction(QStringLiteral("→"), I18n::t(QStringLiteral("Continue")),
                           [this, id = msg.id]() { emit continueRequested(id); });
        }
        if (msg.role != ChatMessage::System)
            makeAction(QStringLiteral("✕"), I18n::t(QStringLiteral("Delete")),
                       [this, id = msg.id]() { emit deleteRequested(id); });
    }
    al->addStretch(1);
    row->addActions(actions, msg.role == ChatMessage::User ? Qt::AlignRight : Qt::AlignLeft);

    if (!msg.streaming) row->addTimestamp();
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
    m_streamingLabel->setText(mdToHtml(m_streamRaw, link, thBg, &m_codeBlocks));
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
        m_reasoningToggle = new QPushButton(QStringLiteral("🧠 ") + I18n::t(QStringLiteral("Reasoning")),
                                            host);
        m_reasoningToggle->setProperty("reasoningToggle", true);
        connect(m_reasoningToggle, &QPushButton::clicked, this, [this]() {
            m_streamReasoningVisible = !m_streamReasoningVisible;
            if (m_streamingReasoning)
                m_streamingReasoning->setVisible(m_streamReasoningVisible);
            m_reasoningToggle->setText(QString(m_streamReasoningVisible ? "🧠 " : "🧠 ") +
                                       I18n::t(QStringLiteral("Reasoning")));
        });
        vl->addWidget(m_reasoningToggle);
        m_streamingReasoning = new QLabel(m_streamReasoning.toHtmlEscaped());
        m_streamingReasoning->setObjectName(QStringLiteral("reasoningText"));
        m_streamingReasoning->setWordWrap(true);
        m_streamingReasoning->setTextInteractionFlags(Qt::TextSelectableByMouse);
        vl->addWidget(m_streamingReasoning);
        // insert above the streaming label: find the row containing it
        const int n = m_rows->count();
        for (int i = 0; i < n; ++i) {
            auto *item = m_rows->itemAt(i);
            auto *w = item->widget();
            if (w && w->objectName() == QStringLiteral("chatRow")) {
                m_rows->insertWidget(i, host);
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
