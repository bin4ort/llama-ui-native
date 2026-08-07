/*
 * main.cpp — v4 native Qt GUI entry.
 *
 * No Qt environment variables are used or required: no QT_QPA_PLATFORM, no
 * QTWEBENGINE_* flags, no launcher script. Theme, DPI and fonts are handled
 * in code; the app runs from a plain environment.
 */
#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("llama-ui-v4"));
    app.setOrganizationName(QStringLiteral("LlamaUI"));

    MainWindow window;
    window.show();
    return app.exec();
}
