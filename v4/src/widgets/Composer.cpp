#include "Composer.h"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QTextEdit>

Composer::Composer(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("composer"));
    setFixedHeight(72);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(6);

    auto *attach = new QPushButton(QStringLiteral("📎"), this);
    attach->setProperty("round", true);
    attach->setToolTip(QStringLiteral("Attach file"));
    layout->addWidget(attach);

    auto *persona = new QPushButton(QStringLiteral("✧"), this);
    persona->setProperty("round", true);
    persona->setToolTip(QStringLiteral("Choose a personality preset"));
    layout->addWidget(persona);

    m_input = new QTextEdit(this);
    m_input->setPlaceholderText(QStringLiteral("Type a message..."));
    m_input->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input->setFixedHeight(40);
    m_input->setTabChangesFocus(false);
    layout->addWidget(m_input, 1);

    m_send = new QPushButton(QStringLiteral("↑"), this);
    m_send->setProperty("send", true);
    layout->addWidget(m_send);

    auto trySend = [this]() {
        const QString t = text().trimmed();
        if (!t.isEmpty()) emit sendRequested(t);
    };
    connect(m_send, &QPushButton::clicked, this, trySend);
    // Enter sends; Shift+Enter = newline (spec: sendOnEnter)
    m_input->installEventFilter(this);
    m_send->setEnabled(false);
    connect(m_input, &QTextEdit::textChanged, this, [this]() {
        m_send->setEnabled(!text().trimmed().isEmpty());
    });
}

QString Composer::text() const { return m_input->toPlainText(); }

void Composer::clear() { m_input->clear(); }

void Composer::setBusy(bool busy) {
    m_send->setEnabled(!busy && !text().trimmed().isEmpty());
    m_input->setReadOnly(busy);
}

bool Composer::eventFilter(QObject *obj, QEvent *ev) {
    if (obj == m_input && ev->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(ev);
        if (ke->key() == Qt::Key_Return && !(ke->modifiers() & Qt::ShiftModifier)) {
            const QString t = text().trimmed();
            if (!t.isEmpty()) emit sendRequested(t);
            return true;
        }
    }
    return QWidget::eventFilter(obj, ev);
}
