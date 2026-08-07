/*
 * WizardDialog.h — preset wizard: describe a role, the model drafts a
 * detailed system prompt (meta-prompt from UI-SPEC / the web app), review
 * and save.
 */
#pragma once
#include <QDialog>

class Api;
class QTextEdit;
class QLineEdit;

class WizardDialog : public QDialog {
    Q_OBJECT
public:
    WizardDialog(Api *api, const QString &model, QWidget *parent = nullptr);

signals:
    void presetCreated(const QString &name, const QString &description,
                       const QString &content);

private:
    Api *m_api;
    QString m_model;
    QTextEdit *m_request;
    QTextEdit *m_draft;
    QLineEdit *m_name;
    QLineEdit *m_description;
    bool m_generated = false;

    void generate();
    void save();
};
