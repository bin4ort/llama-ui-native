#include "MessageView.h"
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QRegularExpression>

namespace {
/** Minimal markdown -> HTML (UI-SPEC §07): code fences, headings, bold,
 *  italic, inline code, lists, links, paragraphs. */
QString mdToHtml(const QString &md) {
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
    // bold / italic / inline code / links
    html.replace(QRegularExpression(R"(\*\*([^*]+)\*\*)"), QStringLiteral("<b>\1</b>"));
    html.replace(QRegularExpression(R"(\*([^*\n]+)\*)"), QStringLiteral("<i>\1</i>"));
    html.replace(QRegularExpression(R"(`([^`]+)`)"),
                 QStringLiteral("<code style='background-color:rgba(128,128,128,0.15);"
                                "border-radius:4px;padding:1px 4px;font-family:monospace'>\1</code>"));
    html.replace(QRegularExpression(R"(\[([^\]]+)\]\(([^)\s]+)\))"),
                 QStringLiteral("<a href='\2' style='color:var(--link)'>\1</a>"));
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
                        tbl += QStringLiteral("<th bgcolor='#666666'>") + h.trimmed() +
                               QStringLiteral("</th>");
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
                    out += QStringLiteral("<ul>");
                    for (const auto &it : items)
                        out += QStringLiteral("<li>") + it + QStringLiteral("</li>");
                    out += QStringLiteral("</ul>");
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

void MessageView::showGreeting() {
    m_layout->addStretch(1);
    auto *title = new QLabel(QStringLiteral("What can I help you with?"));
    title->setObjectName(QStringLiteral("greetingTitle"));
    title->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(title);
    auto *sub = new QLabel(QStringLiteral("Start a conversation with your local model."));
    sub->setObjectName(QStringLiteral("greetingSub"));
    sub->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(sub);
    m_layout->addStretch(1);
}

void MessageView::addMessage(const ChatMessage &msg) {
    if (m_layout->count() > 0) {
        // drop greeting stretches/labels once real content arrives
        while (m_layout->count()) {
            auto *item = m_layout->takeAt(0);
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
    }
    QLabel *label = nullptr;
    if (msg.role == ChatMessage::User) {
        label = new QLabel(msg.content);
        label->setObjectName(QStringLiteral("userBubble"));
        label->setWordWrap(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_layout->addWidget(label, 0, Qt::AlignRight);
    } else if (msg.role == ChatMessage::System) {
        label = new QLabel(mdToHtml(msg.content));
        label->setObjectName(QStringLiteral("systemBubble"));
        label->setTextFormat(Qt::RichText);
        label->setWordWrap(true);
        m_layout->addWidget(label, 0, Qt::AlignRight);
    } else {
        label = new QLabel(mdToHtml(msg.content));
        label->setObjectName(QStringLiteral("assistantText"));
        label->setWordWrap(true);
        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_layout->addWidget(label);
    }
    if (msg.streaming) {
        m_streamingLabel = label;
        m_streamRaw.clear();
        label->setText(QStringLiteral("…"));
    }
    // keep the view pinned to the bottom
    QTimer::singleShot(0, this, [this]() {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    });
}

QString MessageView::lastAssistantHtml() const {
    return m_streamingLabel ? m_streamingLabel->text() : QString();
}

void MessageView::updateStreaming(const QString &delta) {
    if (!m_streamingLabel) return;
    m_streamRaw += delta;
    m_streamingLabel->setText(mdToHtml(m_streamRaw));
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void MessageView::clear() {
    while (m_layout->count()) {
        auto *item = m_layout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    m_streamingLabel = nullptr;
    showGreeting();
}
