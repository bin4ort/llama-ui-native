/*
 * main.cpp — v4 native Qt GUI entry.
 *
 * No Qt environment variables are used or required: no QT_QPA_PLATFORM, no
 * QTWEBENGINE_* flags, no launcher script. Theme, DPI and fonts are handled
 * in code; the app runs from a plain environment.
 */
#include "MainWindow.h"
#include <QApplication>
#include <QGuiApplication>
#include <QTimer>
#include <QScreen>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("llama-ui-v4"));
    app.setOrganizationName(QStringLiteral("LlamaUI"));

    MainWindow window;
    const QStringList a = app.arguments();
    if (a.contains(QStringLiteral("--expand-sidebar")))
        window.expandSidebarForTest();
    const int thIdx = a.indexOf(QStringLiteral("--theme"));
    if (thIdx != -1 && thIdx + 1 < a.size())
        window.overrideThemeForTest(a.at(thIdx + 1));
    window.show();
    if (a.contains(QStringLiteral("--open-settings")))
        QTimer::singleShot(700, &window, &MainWindow::openSettingsForTest);

    const QStringList args = app.arguments();
    const int idx = args.indexOf(QStringLiteral("--screenshot"));

    // --selftest "<message>" [--screenshot <path>]: E2E — new chat, send a
    // message, wait for the reply, screenshot, exit.
    const int stIdx = args.indexOf(QStringLiteral("--selftest"));
    if (stIdx != -1 && stIdx + 1 < args.size()) {
        const QString shot = (idx != -1 && idx + 1 < args.size()) ? args.at(idx + 1)
                                                                  : QString();
        QTimer::singleShot(900, [&window, &app, &args, stIdx, shot]() {
            QObject::connect(&window, &MainWindow::selfTestDone, &app,
                             [&app]() { app.exit(0); });
            window.runSelfTest(args.at(stIdx + 1), shot);
        });
        return app.exec();
    }

    // --screenshot <path> [--shot-width <px>]: grab the window, downscale
    // and exit (used by the automated vision verification loop — small
    // JPEGs fit the local vision model's context window).
    if (idx != -1 && idx + 1 < args.size()) {
        const int wIdx = args.indexOf(QStringLiteral("--shot-width"));
        const int width = (wIdx != -1 && wIdx + 1 < args.size())
                              ? args.at(wIdx + 1).toInt()
                              : 480;
        QTimer::singleShot(900, [&window, &app, &args, idx, width]() {
            QImage img = window.grab().toImage();
            img = img.scaledToWidth(width, Qt::SmoothTransformation);
            const bool ok = img.save(args.at(idx + 1), "JPG", 65);
            app.exit(ok ? 0 : 1);
        });
        return app.exec();
    }

    return app.exec();
}
