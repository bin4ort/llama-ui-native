#include "ModelsSelector.h"
#include "../i18n.h"
#include <QDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPainter>
#include <QVBoxLayout>

ModelsSelector::ModelsSelector(QWidget *parent) : QToolButton(parent) {
    setObjectName(QStringLiteral("modelSelect"));
    setCursor(Qt::PointingHandCursor);
    setToolTip(I18n::t(QStringLiteral("Model")));
    connect(this, &QToolButton::clicked, this, &ModelsSelector::openPopup);
}

void ModelsSelector::setModels(const QStringList &displayItems, const QStringList &rawIds,
                               const QString &currentRaw) {
    m_display = displayItems;
    m_raw = rawIds;
    m_currentRaw = currentRaw;
    update();
}

void ModelsSelector::paintEvent(QPaintEvent *) {
    // trigger: short name (without tags) + chevron
    QString text = m_currentRaw;
    const int slash = text.lastIndexOf('/');
    if (slash >= 0) text = text.mid(slash + 1);
    text.replace(QStringLiteral(".gguf"), QString());
    if (text.size() > 28) text = text.left(27) + QStringLiteral("…");

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QFont f = font();
    f.setPixelSize(12);
    p.setFont(f);
    const QPalette pal = palette();
    p.setPen(pal.color(QPalette::WindowText));
    p.drawText(rect().adjusted(8, 0, -18, 0), Qt::AlignVCenter | Qt::AlignLeft, text);
    // chevron
    const QRectF c = QRectF(width() - 14, height() / 2.0 - 2, 8, 5);
    QPolygonF tri;
    tri << c.topLeft() << c.topRight() << QPointF(c.center().x(), c.bottom());
    p.setBrush(pal.color(QPalette::WindowText));
    p.setPen(Qt::NoPen);
    p.drawPolygon(tri);
}

void ModelsSelector::openPopup() {
    auto *popup = new QDialog(this);
    popup->setObjectName(QStringLiteral("appDialog"));
    popup->setModal(true);
    popup->resize(340, 420);
    auto *lay = new QVBoxLayout(popup);
    lay->setContentsMargins(12, 12, 12, 12);
    lay->setSpacing(8);

    m_search = new QLineEdit(popup);
    m_search->setPlaceholderText(I18n::t(QStringLiteral("Search models...")));
    lay->addWidget(m_search);

    m_list = new QListWidget(popup);
    m_list->setObjectName(QStringLiteral("modelList"));
    lay->addWidget(m_list, 1);

    auto populate = [this](const QString &needle) {
        m_list->clear();
        QString lastSection;
        for (int i = 0; i < m_display.size(); ++i) {
            if (!needle.isEmpty() &&
                !m_display.at(i).contains(needle, Qt::CaseInsensitive))
                continue;
            // section: Loaded (current) / Available (v2 grouping)
            const bool loaded = m_raw.at(i) == m_currentRaw;
            const QString section = loaded
                                        ? I18n::t(QStringLiteral("Loaded models"))
                                        : I18n::t(QStringLiteral("Available models"));
            if (section != lastSection) {
                auto *header = new QListWidgetItem(section);
                header->setFlags(Qt::NoItemFlags);
                m_list->addItem(header);
                lastSection = section;
            }
            auto *item = new QListWidgetItem(m_display.at(i));
            item->setData(Qt::UserRole, m_raw.at(i));
            if (loaded) item->setIcon(style()->standardIcon(QStyle::SP_DialogYesButton));
            m_list->addItem(item);
        }
    };
    populate(QString());
    connect(m_search, &QLineEdit::textChanged, popup, [populate](const QString &q) {
        populate(q);
    });
    connect(m_list, &QListWidget::itemClicked, this, [this, popup](QListWidgetItem *item) {
        const QString raw = item->data(Qt::UserRole).toString();
        if (!raw.isEmpty()) {
            m_currentRaw = raw;
            update();
            emit modelSelected(raw);
            popup->accept();
        }
    });
    popup->exec();
    popup->deleteLater();
}
