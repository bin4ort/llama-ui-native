#include "Composer.h"
#include "../Config.h"
#include "../i18n.h"
#include "ContextGauge.h"
#include <QAbstractTextDocumentLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTextDocument>
#include <QTextEdit>
#include <QVBoxLayout>

QString Composer::formatSize(qint64 bytes) {
    if (bytes < 1024) return QStringLiteral("%1 B").arg(bytes);
    if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    if (bytes < 1024 * 1024 * 1024)
        return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    return QStringLiteral("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 1);
}

Composer::Composer(QWidget *parent) : QWidget(parent) {
    setObjectName(QStringLiteral("composer"));
    setMaximumWidth(768); // spec: max-w-3xl, centered floating bar

    m_outer = new QVBoxLayout(this);
    m_outer->setContentsMargins(12, 8, 12, 10);
    m_outer->setSpacing(4);

    // attachment chips row (UI-SPEC §4.2)
    m_chipsRow = new QHBoxLayout;
    m_chipsRow->setContentsMargins(0, 0, 0, 0);
    m_chipsRow->setSpacing(6);
    m_outer->addLayout(m_chipsRow);

    auto *bar = new QHBoxLayout;
    bar->setContentsMargins(0, 0, 0, 0);
    bar->setSpacing(6);

    auto *attach = new QPushButton(QStringLiteral("📎"), this);
    attach->setProperty("round", true);
    attach->setToolTip(I18n::t(QStringLiteral("Attach file")));
    connect(attach, &QPushButton::clicked, this, &Composer::attachRequested);
    bar->addWidget(attach);

    m_persona = new QPushButton(QStringLiteral("✦ Default ▾"), this);
    m_persona->setProperty("persona", true);
    m_persona->setToolTip(I18n::t(QStringLiteral("Choose a personality preset")));
    connect(m_persona, &QPushButton::clicked, this, &Composer::personaRequested);
    bar->addWidget(m_persona);

    m_input = new QTextEdit(this);
    m_input->setPlaceholderText(I18n::t(QStringLiteral("Type a message...")));
    m_input->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input->setTabChangesFocus(false);
    bar->addWidget(m_input, 1);

    m_send = new QPushButton(QStringLiteral("↑"), this);
    m_send->setProperty("send", true);

    // right cluster (v2 ChatFormActions order): gauge -> model -> send
    m_gauge = new ContextGauge(this);
    m_gauge->setVisible(false); // hidden until tokens processed (v2)
    bar->addWidget(m_gauge);

    m_modelSelect = new QComboBox(this);
    m_modelSelect->setProperty("modelSelect", true);
    m_modelSelect->setMaximumWidth(220);
    m_modelSelect->setMinimumWidth(140);
    m_modelSelect->setToolTip(I18n::t(QStringLiteral("Model")));
    connect(m_modelSelect, &QComboBox::currentTextChanged, this,
            [this](const QString &) {
                const QString raw = m_modelSelect->currentData().toString();
                if (!raw.isEmpty()) emit modelSelected(raw);
            });
    bar->addWidget(m_modelSelect);

    bar->addWidget(m_send);
    m_outer->addLayout(bar);

    auto trySend = [this]() {
        if (m_busy) {
            emit stopRequested();
            return;
        }
        const QString t = text().trimmed();
        if (!t.isEmpty() || !m_attachments.isEmpty()) emit sendRequested(t);
    };
    connect(m_send, &QPushButton::clicked, this, trySend);
    // Enter sends (sendOnEnter setting); Shift+Enter = newline
    m_input->installEventFilter(this);
    m_send->setEnabled(false);
    connect(m_input, &QTextEdit::textChanged, this, [this]() {
        m_send->setEnabled(!m_busy && (!text().trimmed().isEmpty() || !m_attachments.isEmpty()));
        updateHeight();
    });
    updateHeight();
}

QString Composer::text() const { return m_input->toPlainText(); }

void Composer::clear() {
    m_input->clear();
    m_attachments = QJsonArray();
    rebuildChips();
    m_send->setEnabled(!m_busy);
}

void Composer::focusInput() {
    m_input->setFocus();
    m_input->moveCursor(QTextCursor::End);
}

void Composer::setTextForEdit(const QString &text) {
    m_input->setPlainText(text);
    focusInput();
}

void Composer::addAttachment(const QJsonObject &att) {
    m_attachments.append(att);
    rebuildChips();
    m_send->setEnabled(!m_busy);
    emit attachmentsChanged();
}

void Composer::setAttachments(const QJsonArray &atts) {
    m_attachments = atts;
    rebuildChips();
    m_send->setEnabled(!m_busy && (!text().trimmed().isEmpty() || !m_attachments.isEmpty()));
}

void Composer::setModels(const QStringList &displayItems, const QStringList &rawIds,
                         const QString &currentRaw) {
    m_modelSelect->blockSignals(true);
    m_modelSelect->clear();
    for (int i = 0; i < displayItems.size(); ++i) {
        m_modelSelect->addItem(displayItems.at(i),
                               i < rawIds.size() ? rawIds.at(i) : displayItems.at(i));
    }
    const int idx = qMax(0, m_modelSelect->findData(currentRaw));
    m_modelSelect->setCurrentIndex(idx);
    m_modelSelect->blockSignals(false);
    emit modelSelected(m_modelSelect->currentData().toString());
}

QString Composer::model() const {
    return m_modelSelect->currentData().toString();
}

void Composer::setPersonaName(const QString &name) {
    m_persona->setText(QStringLiteral("✦ ") + name + QStringLiteral(" ▾"));
    m_persona->setProperty("active", !name.isEmpty() && name != I18n::t(QStringLiteral("Default")));
    m_persona->style()->unpolish(m_persona);
    m_persona->style()->polish(m_persona);
}

void Composer::setContextUsage(qint64 promptChars, qint64 completionChars, int maxCtx) {
    if (!m_gauge) return;
    m_gauge->setVisible(promptChars > 0 || completionChars > 0); // v2: only with tokens
    m_gauge->setUsage(promptChars, completionChars, maxCtx);
}

void Composer::rebuildChips() {
    while (m_chipsRow->count()) {
        auto *item = m_chipsRow->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }
    for (int i = 0; i < m_attachments.size(); ++i) {
        const auto obj = m_attachments.at(i).toObject();
        const QString name = obj.value(QStringLiteral("name")).toString();
        const bool image = obj.value(QStringLiteral("image")).toBool();
        auto *chip = new QWidget(this);
        chip->setObjectName(QStringLiteral("attChip"));
        auto *hl = new QHBoxLayout(chip);
        hl->setContentsMargins(6, 4, 6, 4);
        hl->setSpacing(6);
        if (image) {
            // real thumbnail (v2 ChatAttachmentsListItemThumbnailImage, h-24)
            const QByteArray b64 = obj.value(QStringLiteral("data")).toString().toLatin1();
            QPixmap pm;
            pm.loadFromData(QByteArray::fromBase64(b64), "PNG");
            if (!pm.isNull()) {
                auto *thumb = new QLabel(chip);
                thumb->setPixmap(pm.scaled(64, 64, Qt::KeepAspectRatio,
                                           Qt::SmoothTransformation));
                thumb->setObjectName(QStringLiteral("attThumb"));
                thumb->setCursor(Qt::PointingHandCursor);
                thumb->installEventFilter(this);
                hl->addWidget(thumb);
            }
        } else {
            auto *icon = new QLabel(QStringLiteral("📄"), chip);
            icon->setObjectName(QStringLiteral("attIcon"));
            icon->setFixedSize(32, 32);
            icon->setCursor(Qt::PointingHandCursor);
            icon->installEventFilter(this);
            hl->addWidget(icon);
        }
        auto *txt = new QVBoxLayout;
        txt->setContentsMargins(0, 0, 0, 0);
        txt->setSpacing(0);
        auto *lbl = new QLabel(name, chip);
        lbl->setObjectName(QStringLiteral("attChipLabel"));
        txt->addWidget(lbl);
        const qint64 size = obj.value(QStringLiteral("size")).toDouble();
        if (size > 0) {
            auto *sizeLbl = new QLabel(formatSize(size), chip);
            sizeLbl->setObjectName(QStringLiteral("attSizeLabel"));
            txt->addWidget(sizeLbl);
        }
        hl->addLayout(txt);
        auto *rm = new QPushButton(QStringLiteral("✕"), chip);
        rm->setProperty("ghost", true);
        rm->setFixedSize(18, 18);
        connect(rm, &QPushButton::clicked, this, [this, i]() {
            m_attachments.removeAt(i);
            rebuildChips();
            m_send->setEnabled(!m_busy && (!text().trimmed().isEmpty() || !m_attachments.isEmpty()));
            emit attachmentsChanged();
        });
        hl->addWidget(rm);
        m_chipsRow->addWidget(chip);
    }
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
        m_send->setEnabled(!text().trimmed().isEmpty() || !m_attachments.isEmpty());
        m_input->setReadOnly(false);
    }
}

/** Autosize the textarea (spec §5.1: field-sizing-content, min 64px). */
void Composer::updateHeight() {
    const int docH = qCeil(m_input->document()->documentLayout()->documentSize().height());
    const int h = qBound(40, docH + 8, 160);
    m_input->setFixedHeight(h);
    setFixedHeight(h + 32 + (m_attachments.isEmpty() ? 0 : 28));
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
                if (!t.isEmpty() || !m_attachments.isEmpty()) emit sendRequested(t);
            }
            return true;
        }
        if (!sendOnEnter && enter && ctrl && !shift) {
            if (!m_busy) {
                const QString t = text().trimmed();
                if (!t.isEmpty() || !m_attachments.isEmpty()) emit sendRequested(t);
            }
            return true;
        }
    }
    if (ev->type() == QEvent::MouseButtonRelease) {
        auto *me = static_cast<QMouseEvent *>(ev);
        if (me->button() == Qt::LeftButton) {
            // preview the attachment whose thumbnail/icon was clicked (v2 §4.3)
            QWidget *w = qobject_cast<QWidget *>(obj);
            while (w && w->parentWidget() != this && w->parentWidget()->parentWidget() != this)
                w = w->parentWidget();
            if (w && w->parentWidget() && w->parentWidget()->parentWidget() == this) {
                const int idx = m_chipsRow->indexOf(w->parentWidget());
                if (idx >= 0) emit attachmentPreviewRequested(idx);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}
