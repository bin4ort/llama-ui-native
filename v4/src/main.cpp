/*
 * main.cpp — v4 native Qt GUI entry.
 *
 * No Qt environment variables are used or required: no QT_QPA_PLATFORM, no
 * QTWEBENGINE_* flags, no launcher script. Theme, DPI and fonts are handled
 * in code; the app runs from a plain environment.
 */
#include "MainWindow.h"
#include <QApplication>
#include <cstdio>
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
    if (a.contains(QStringLiteral("--open-settings"))) {
        int secIdx = -1;
        const int sIdx = a.indexOf(QStringLiteral("--settings-section"));
        if (sIdx != -1 && sIdx + 1 < a.size()) {
            const QStringList secs = {"general", "display", "sampling", "penalties",
                                      "agentic", "developer", "tools", "import-export"};
            secIdx = secs.indexOf(a.at(sIdx + 1));
        }
        QTimer::singleShot(700, &window, [&window, secIdx]() {
            window.openSettingsForTest(secIdx);
        });
    }
    if (a.contains(QStringLiteral("--open-mcp")))
        QTimer::singleShot(700, &window, [&window]() { window.openMcpForTest(); });
    if (a.contains(QStringLiteral("--open-search")))
        QTimer::singleShot(700, &window, [&window]() { window.openSearchForTest(); });

    const QStringList args = app.arguments();
    const int idx = args.indexOf(QStringLiteral("--screenshot"));

    // --pixel-report <x>,<y>: sample the window pixel at (x,y) and print
    // its RGB (deterministic ground truth for the vision loop).
    const int pxIdx = args.indexOf(QStringLiteral("--pixel-report"));
    if (pxIdx != -1 && pxIdx + 1 < args.size()) {
        const QStringList xy = args.at(pxIdx + 1).split(',');
        if (xy.size() == 2) {
            const QPoint pt(xy.at(0).toInt(), xy.at(1).toInt());
            QTimer::singleShot(900, [&window, &app, pt]() {
                const QImage img = window.grab().toImage();
                const QColor c = img.pixelColor(pt);
                // average over a 20x20 patch around the point
                double r = 0, g = 0, b = 0; int n = 0;
                for (int dx = -10; dx <= 10; dx += 2)
                    for (int dy = -10; dy <= 10; dy += 2) {
                        const QColor p = img.pixelColor(pt + QPoint(dx, dy));
                        r += p.red(); g += p.green(); b += p.blue(); ++n;
                    }
                fprintf(stderr, "PIXEL %d,%d = rgb(%d,%d,%d) avg24 rgb(%d,%d,%d)\n",
                        pt.x(), pt.y(), c.red(), c.green(), c.blue(),
                        int(r / n), int(g / n), int(b / n));
                app.exit(0);
            });
            return app.exec();
        }
    }

    // --selftest "<message>" [--attach <file>...] [--screenshot <path>]:
    // E2E — new chat, send a message (with optional attachments), wait for
    // the reply, screenshot, exit.
    const int stIdx = args.indexOf(QStringLiteral("--selftest"));
    if (stIdx != -1 && stIdx + 1 < args.size()) {
        const QString shot = (idx != -1 && idx + 1 < args.size()) ? args.at(idx + 1)
                                                                  : QString();
        QStringList attaches;
        int aIdx = args.indexOf(QStringLiteral("--attach"));
        while (aIdx != -1 && aIdx + 1 < args.size() && !args.at(aIdx + 1).startsWith(QStringLiteral("--"))) {
            attaches << args.at(aIdx + 1);
            aIdx = args.indexOf(QStringLiteral("--attach"), aIdx + 1);
        }
        QTimer::singleShot(900, [&window, &app, &args, stIdx, shot, attaches]() {
            QObject::connect(&window, &MainWindow::selfTestDone, &app,
                             [&app]() { app.exit(0); });
            if (!attaches.isEmpty()) window.attachForTest(attaches);
            window.runSelfTest(args.at(stIdx + 1), shot);
            // --stop-after <ms>: abort the stream mid-flight (stop button
            // equivalent) — verifies the abort path end to end
            const int stopIdx = args.indexOf(QStringLiteral("--stop-after"));
            if (stopIdx != -1 && stopIdx + 1 < args.size()) {
                QTimer::singleShot(args.at(stopIdx + 1).toInt(),
                                   &window, &MainWindow::abortForTest);
            }
        });
        return app.exec();
    }

    // --tool-test "<expr>": run the calculator tool and print the result
    // (verifies the agentic tool execution path without a model)
    const int ttIdx = args.indexOf(QStringLiteral("--tool-test"));
    if (ttIdx != -1 && ttIdx + 1 < args.size()) {
        QTimer::singleShot(500, [&app, &args, ttIdx]() {
            const QString expr = args.at(ttIdx + 1);
            bool ok = false;
            extern double v4SafeEval(const QString &, bool *);
            const double r = v4SafeEval(expr, &ok);
            fprintf(stderr, "TOOLTEST %s = %s\n", expr.toUtf8().constData(),
                    ok ? qPrintable(QString::number(r, 'g', 12)) : "ERROR");
            app.exit(ok ? 0 : 1);
        });
        return app.exec();
    }

    // --geom-dump: print widget tree geometry (dialog/page hierarchy)
    if (args.contains(QStringLiteral("--geom-dump"))) {
        QTimer::singleShot(2500, [&window, &app]() {
            QWidget *target = qApp->activeModalWidget();
            if (!target) target = &window;
            // activate the layout of the current stack page first
            QCoreApplication::processEvents();
            std::function<void(QWidget *, int)> dump;
            dump = [&dump](QWidget *w, int depth) {
                const QRect r = w->geometry();
                // print both parent-relative and card-relative coords for
                // the settings field label/checkbox debugging
                QWidget *card = nullptr;
                QWidget *p = w->parentWidget();
                while (p) {
                    if (p->objectName() == QStringLiteral("settingsCard")) { card = p; break; }
                    p = p->parentWidget();
                }
                fprintf(stderr, "%*s%s[%s] %d,%d %dx%d vis=%d%s sh=%dx%d\n", depth * 2, "",
                        w->objectName().toUtf8().constData(),
                        w->metaObject()->className(),
                        r.x(), r.y(), r.width(), r.height(), w->isVisible() ? 1 : 0,
                        card ? " [IN-CARD]" : "",
                        w->sizeHint().width(), w->sizeHint().height());
                if (card && (w->metaObject()->className() == QStringLiteral("QLabel") ||
                             w->metaObject()->className() == QStringLiteral("QCheckBox"))) {
                    const QPoint rel = w->mapTo(card, QPoint(0, 0));
                    const QPoint g = w->mapToGlobal(QPoint(0, 0));
                    const QPoint cg = card->mapToGlobal(QPoint(0, 0));
                    fprintf(stderr, "      REL-TO-CARD: %s at %d,%d global=%d,%d cardGlobal=%d,%d sizeHint=%dx%d\n",
                            w->metaObject()->className(), rel.x(), rel.y(),
                            g.x(), g.y(), cg.x(), cg.y(),
                            w->sizeHint().width(), w->sizeHint().height());
                }
                for (QObject *c : w->children())
                    if (auto *cw = qobject_cast<QWidget *>(c))
                        dump(cw, depth + 1);
            };
            dump(target, 0);
            if (auto *col = window.chatCol()) {
                fprintf(stderr, "--- chatCol items ---\n");
                for (int i = 0; i < col->count(); ++i) {
                    QLayoutItem *it = col->itemAt(i);
                    QWidget *w = it->widget();
                    fprintf(stderr, "  [%d] %s rect=%dx%d h=%d stretch=%d\n", i,
                            w ? w->objectName().toUtf8().constData() : "(spacer)",
                            it->geometry().width(), it->geometry().height(),
                            it->geometry().y(),
                            col->stretch(i));
                }
            }
            app.exit(0);
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
            // grab the active modal dialog when one is open (settings,
            // MCP page, search) — window.grab() misses separate windows
            QWidget *target = qApp->activeModalWidget();
            if (!target) target = &window;
            QImage img = target->grab().toImage();
            img = img.scaledToWidth(width, Qt::SmoothTransformation);
            const QString ext = args.at(idx + 1).endsWith(QStringLiteral(".png"),
                                                           Qt::CaseInsensitive)
                                    ? QStringLiteral("PNG")
                                    : QStringLiteral("JPG");
            const bool ok = img.save(args.at(idx + 1), ext.toUtf8().constData(),
                                     ext == QLatin1String("PNG") ? -1 : 65);
            app.exit(ok ? 0 : 1);
        });
        return app.exec();
    }

    return app.exec();
}
