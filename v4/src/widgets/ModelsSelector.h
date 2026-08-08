/*
 * ModelsSelector.h — v2 ModelsSelectorDropdown (UI-SPEC §11.5): a trigger
 * showing the short model name + chevron; clicking opens a popup with a
 * search field and grouped sections (Loaded / Available) with
 * quantization + modality tags.
 */
#pragma once
#include <QToolButton>

class QLineEdit;
class QListWidget;
class QVBoxLayout;

class ModelsSelector : public QToolButton {
    Q_OBJECT
public:
    explicit ModelsSelector(QWidget *parent = nullptr);

    /** displayItems carry tags; rawIds are the real model ids. */
    void setModels(const QStringList &displayItems, const QStringList &rawIds,
                   const QString &currentRaw);
    QString model() const { return m_currentRaw; }

signals:
    void modelSelected(const QString &rawId);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void openPopup();
    void selectItem(int row);

    QStringList m_display;
    QStringList m_raw;
    QString m_currentRaw;
    QLineEdit *m_search = nullptr;
    QListWidget *m_list = nullptr;
};
