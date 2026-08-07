/*
 * SettingsDialog.h — minimal spec settings (registry fields): language,
 * theme (with all 11 palettes), server endpoint, sampling inputs.
 */
#pragma once
#include <QDialog>
#include <QJsonObject>

class QComboBox;
class QLineEdit;
class QPushButton;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QJsonObject cfg, QWidget *parent = nullptr);

    QJsonObject config() const { return m_cfg; }

signals:
    void configChanged(const QJsonObject &cfg);

private:
    QJsonObject m_cfg;
    QComboBox *m_lang = nullptr;
    QComboBox *m_theme = nullptr;
    QLineEdit *m_endpoint = nullptr;
    QLineEdit *m_temperature = nullptr;
    QLineEdit *m_topK = nullptr;
    QLineEdit *m_topP = nullptr;
    QLineEdit *m_systemMessage = nullptr;

    void apply();
};
