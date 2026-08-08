#include "Composer.h"
#include "../Config.h"
#include "../i18n.h"
#include <QAbstractTextDocumentLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QTextDocument>
#include <QTextEdit>

Composer::Composer(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("composer"));
    setMaximumWidth(768); // spec: max-w-3xl, centered floating bar

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(6);

    auto *attach = new QPushButton(QStringLiteral("📎"), this);
    attach->setProperty("round", true);
    attach->setToolTip(I18n::t(QStringLiteral("Attach file")));
    layout->addWidget(attach);

    m_persona = new QPushButton(QStringLiteral("✧"), this);
    m_persona->setProperty("round", true);
    m_persona->setToolTip(I18n::t(QStringLiteral("Choose a personality preset")));
    connect(m_persona, &QPushButton::clicked, this, &Composer::personaRequested);
    layout->addWidget(m_persona);

    m_input = new QTextEdit(this);
    m_input->setPlaceholderText(I18n::t(QStringLiteral("Type a message...")));
    m_input->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input->setTabChangesFocus(false);
    layout->addWidget(m_input, 1);

    m_send = new QPushButton(QStringLiteral("↑"), this);
    m_send->setProperty("send", true);
    layout->addWidget(m_send);

    auto trySend = [this]() {
        if (m_busy) {
            emit stopRequested();
            return;
        }
        const QString t = text().trimmed();
        if (!t.isEmpty()) emit sendRequested(t);
    };
    connect(m_send, &QPushButton::clicked, this, trySend);
    // Enter sends (sendOnEnter setting); Shift+Enter = newline
    m_input->installEventFilter(this);
    m_send->setEnabled(false);
    connect(m_input, &QTextEdit::textChanged, this, [this]() {
        m_send->setEnabled(!m_busy && !text().trimmed().isEmpty());
        updateHeight();
    });
    updateHeight();
}

QString Composer::text() const { return m_input->toPlainText(); }

void Composer::clear() { m_input->clear(); }

void Composer::focusInput() {
    m_input->setFocus();
    m_input->moveCursor(QTextCursor::End);
}

void Composer::setTextForEdit(const QString &text) {
    m_input->setPlainText(text);
    focusInput();
}

void Composer::setBusy(bool busy) {
    m_busy = busy;
    if (busy) {
        // stop button replaces send while streaming (UI-SPEC §5.6)
        m_send->setText(QStringLiteral("■"));
        m_send->setToolTip(I18n::t(QStringLiteral("Stop")));
        m_send->setEnabled(true);
        m_input->setReadOnly(true);
    } else {
        m_send->setText(QStringLiteral("↑"));
        m_send->setToolTip(I18n::t(QStringLiteral("Send")));
        m_send->setEnabled(!text().trimmed().isEmpty());
        m_input->setReadOnly(false);
    }
}

/** Autosize the textarea (spec §5.1: field-sizing-content, min 64px). */
void Composer::updateHeight() {
    const int docH = qCeil(m_input->document()->documentLayout()->documentSize().height());
    const int h = qBound(40, docH + 8, 160);
    m_input->setFixedHeight(h);
    setFixedHeight(h + 32);
}

bool Composer::eventFilter(QObject *obj, QEvent *ev) {
    if (obj == m_input && ev->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(ev);
        const bool sendOnEnter = Config::value(QStringLiteral("sendOnEnter")).toBool();
        const bool enter = ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter;
        const bool shift = ke->modifiers() & Qt::ShiftModifier;
        const bool ctrl = ke->modifiers() & Qt::ControlModifier;
        if (sendOnEnter && enter && !shift) {
            if (!m_busy) {
                const QString t = text().trimmed();
                if (!t.isEmpty()) emit sendRequested(t);
            }
            return true;
        }
        if (!sendOnEnter && enter && ctrl && !shift) {
            if (!m_busy) {
                const QString t = text().trimmed();
                if (!t.isEmpty()) emit sendRequested(t);
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj, ev);
}
