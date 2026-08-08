/*
 * SettingsDialog.h — full settings dialog (registry-driven, UI-SPEC §09):
 * tabs per section with every v0.4.3 field.
 */
#pragma once
#include <QDialog>
#include <QJsonObject>
#include <QList>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QPlainTextEdit;
class QStackedWidget;
class QListWidget;

struct SettingField {
    QString key;
    QString label;
    QString type; // input / checkbox / textarea
    bool positiveInt = false;
    QString help;
};

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QJsonObject cfg, QWidget *parent = nullptr);

    QJsonObject config() const { return m_cfg; }

signals:
    void configChanged(const QJsonObject &cfg);

private:
    QWidget *buildSection(const QString &title, const QList<SettingField> &fields,
                          bool useComboLang = false, bool useComboTheme = false);
    QWidget *buildImportExport();
    void apply();

    QJsonObject m_cfg;
    QStackedWidget *m_stack = nullptr;
    QListWidget *m_navList = nullptr;
    QList<QPair<QString, QWidget *>> m_controls; // field key -> control
};
