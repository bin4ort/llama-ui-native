#include "MessageView.h"
#include <QLabel>
#include <QScrollBar>
#include <QTimer>
#include <QVBoxLayout>

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
        label = new QLabel(msg.content);
        label->setObjectName(QStringLiteral("systemBubble"));
        label->setWordWrap(true);
        m_layout->addWidget(label, 0, Qt::AlignRight);
    } else {
        label = new QLabel(msg.content);
        label->setObjectName(QStringLiteral("assistantText"));
        label->setWordWrap(true);
        label->setTextFormat(Qt::RichText);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_layout->addWidget(label);
    }
    if (msg.streaming) {
        m_streamingLabel = label;
        label->setText(QStringLiteral("…"));
    }
    // keep the view pinned to the bottom
    QTimer::singleShot(0, this, [this]() {
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    });
}

void MessageView::updateStreaming(const QString &delta) {
    if (!m_streamingLabel) return;
    m_streamingLabel->setText(m_streamingLabel->text() + delta);
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
