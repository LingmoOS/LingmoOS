// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window.h"

#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include <DAccessibilityChecker>
#endif

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QTranslator>
#include <QTimer>
#include <QDebug>

#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
DWIDGET_USE_NAMESPACE
#endif

#define DBUS_SERV "org.deepin.dde.LowPower1"
#define DBUS_PATH "/org/deepin/dde/LowPower1"
#define DBUS_IFCE "org.deepin.dde.LowPower1"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    a.installTranslator(&translator);

    QCommandLineOption raiseOption("raise", "raise the main window.");
    QCommandLineOption quitOption("quit", "quit all dde-lowpower instances.");

    // command line parser
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(raiseOption);
    parser.addOption(quitOption);
    parser.process(a);

    Window w;
    LowPowerAdaptor adaptor(&w);
    QDBusInterface iface(DBUS_SERV,
                         DBUS_PATH,
                         DBUS_IFCE,
                         QDBusConnection::sessionBus());

    bool result = QDBusConnection::sessionBus().registerService(DBUS_SERV);
    if (result) {
        if (parser.isSet(quitOption)) {
            return 0;
        } else {
            w.show();
            w.grabMouse();
            if (!w.SleepLock()) {
               w.grabKeyboard();
            }

            QDBusConnection::sessionBus().registerObject(DBUS_PATH, &w);
        }
    } else {
        if (parser.isSet(raiseOption)) {
            iface.call("Raise");
        } else if (parser.isSet(quitOption)) {
            iface.call("Quit");
        }

        QTimer::singleShot(500, &a, SLOT(quit()));
    }

#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    DAccessibilityChecker checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif

    return a.exec();
}
