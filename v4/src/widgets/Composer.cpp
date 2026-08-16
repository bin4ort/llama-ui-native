#include "Composer.h"
#include "../Config.h"
#include "../Icons.h"
#include "../i18n.h"
#include "ContextGauge.h"
#include "ModelsSelector.h"
#include <QAbstractTextDocumentLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QStyle>
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
    // QSS backgrounds on plain QWidget subclasses need this attribute
    setAttribute(Qt::WA_StyledBackground);
    setFixedWidth(768); // v2: max-w-3xl form

    m_outer = new QVBoxLayout(this);
    m_outer->setContentsMargins(12, 13, 12, 13); // v2: inner p-3 (13px vertical)
    m_outer->setSpacing(12); // v2: textarea -> buttons gap

    // attachment chips row (UI-SPEC §4.2)
    m_chipsRow = new QHBoxLayout;
    m_chipsRow->setContentsMargins(0, 0, 0, 0);
    m_chipsRow->setSpacing(6);
    m_outer->addLayout(m_chipsRow);

    // v2 ChatForm: textarea row on top (min-h-12, px-5), buttons row below
    m_input = new QTextEdit(this);
    m_input->setPlaceholderText(I18n::t(QStringLiteral("Type a message...")));
    m_input->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_input->setTabChangesFocus(false);
    m_outer->addWidget(m_input);

    auto *bar = new QHBoxLayout;
    bar->setContentsMargins(0, 0, 0, 0); // v2: buttons hug the form padding
    bar->setSpacing(8);

    m_attach = new QPushButton(this);
    m_attach->setProperty("round", true);
    m_attach->setIconSize(QSize(18, 18));
    m_attach->setFixedSize(32, 32); // v2: ChatFormActionAdd h-8 w-8
    // v2 ChatFormActionAdd: Plus icon, tooltip lists what can be added
    m_attach->setToolTip(I18n::t(QStringLiteral("Add files, prompts, tools or MCP Servers")));
    connect(m_attach, &QPushButton::clicked, this, &Composer::attachRequested);
    bar->addWidget(m_attach);

    m_persona = new QPushButton(QStringLiteral("Default ▾"), this);
    m_persona->setProperty("persona", true);
    m_persona->setIconSize(QSize(16, 16));
    m_persona->setFixedHeight(32); // v2: ChatFormActionsPersona
    m_persona->setToolTip(I18n::t(QStringLiteral("Choose a personality preset")));
    connect(m_persona, &QPushButton::clicked, this, &Composer::personaRequested);
    bar->addWidget(m_persona);
    bar->addStretch(1);

    // right cluster (v2 ChatFormActions order): gauge -> model -> send
    m_gauge = new ContextGauge(this);
    m_gauge->setVisible(false); // hidden until tokens processed (v2)
    bar->addWidget(m_gauge);

    m_modelSelect = new ModelsSelector(this);
    m_modelSelect->setFixedSize(188, 24); // v2: model chip h-6
    connect(m_modelSelect, &ModelsSelector::modelSelected, this,
            &Composer::modelSelected);
    bar->addWidget(m_modelSelect);

    m_send = new QPushButton(this);
    m_send->setProperty("send", true);
    m_send->setIconSize(QSize(22, 22));
    m_send->setFixedSize(32, 32); // v2: send h-8 w-8
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
    applyIcons();
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
    m_modelSelect->setModels(displayItems, rawIds, currentRaw);
    emit modelSelected(currentRaw.isEmpty() ? (rawIds.isEmpty() ? QString() : rawIds.first())
                                            : currentRaw);
}

QString Composer::model() const {
    return m_modelSelect->model();
}

void Composer::setPersonaName(const QString &name) {
    m_persona->setText(name + QStringLiteral(" ▾"));
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
            auto *icon = new QLabel(chip);
            icon->setObjectName(QStringLiteral("attIcon"));
            icon->setPixmap(Icons::get(QStringLiteral("file-text"), Icons::color())
                                .pixmap(20, 20));
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
        auto *rm = new QPushButton(chip);
        rm->setProperty("ghost", true);
        rm->setIcon(Icons::get(QStringLiteral("x"), Icons::color()));
        rm->setIconSize(QSize(12, 12));
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
        m_send->setIcon(Icons::get(QStringLiteral("square-filled"), Icons::color()));
        m_send->setToolTip(I18n::t(QStringLiteral("Stop")));
        m_send->setEnabled(true);
        m_input->setReadOnly(true);
    } else {
        m_send->setIcon(Icons::get(QStringLiteral("arrow-up"), Icons::color()));
        m_send->setToolTip(I18n::t(QStringLiteral("Send")));
        m_send->setEnabled(!text().trimmed().isEmpty() || !m_attachments.isEmpty());
        m_input->setReadOnly(false);
    }
}

void Composer::applyIcons() {
    const QColor c = Icons::color();
    if (m_attach) m_attach->setIcon(Icons::get(QStringLiteral("plus"), c));
    m_persona->setIcon(Icons::get(QStringLiteral("sparkles"), c));
    m_send->setIcon(Icons::get(m_busy ? QStringLiteral("square")
                                      : QStringLiteral("arrow-up"), c));
}

/** Autosize the textarea (spec §5.1: field-sizing-content, min 64px). */
void Composer::updateHeight() {
    const int docH = qCeil(m_input->document()->documentLayout()->documentSize().height());
    const int h = qBound(48, docH + 8, 160);
    m_input->setFixedHeight(h);
    // v2: form = 13px pad + 48 textarea + 12 gap + 32 buttons + 13px pad = 118
    setFixedHeight(h + 70);
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
