/*
 * SettingsDialog.h — full settings dialog (registry-driven, UI-SPEC §09):
 * tabs per section with every v0.4.3 field.
 */
#pragma once
#include <QDialog>
#include <QJsonObject>
#include <QList>
#include "../Theme.h"

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
    QString dependsOn; // v2: field hidden when the dep config is falsy
    bool dependsOnNegated = false; // v2: field hidden when dep is truthy
    bool experimental = false;     // v2: flask-conical badge after the label
};

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QJsonObject cfg, const QJsonObject &props,
                            const Theme::Tokens &tokens,
                            bool sidebarExpanded,
                            QWidget *parent = nullptr);

    QJsonObject config() const { return m_cfg; }
    void setSection(int index);

signals:
    void configChanged(const QJsonObject &cfg);
    void clearAllConversationsRequested();

private:
    QWidget *buildSection(const QString &title, const QList<SettingField> &fields,
                          bool useComboLang = false, bool useComboTheme = false);
    QWidget *buildTools();
    QWidget *buildImportExport();
    void apply();
    void rebuildCurrentSection();

    QJsonObject m_cfg;
    QJsonObject m_props;
    Theme::Tokens m_tokens;
    QStackedWidget *m_stack = nullptr;
    QWidget *m_buttons = nullptr;
    bool m_sidebarExpanded = false;
    QListWidget *m_navList = nullptr;
    QList<QPair<QString, QWidget *>> m_controls; // field key -> control
};
