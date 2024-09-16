// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "touchscreensetting.h"
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include "../common/accessibilitycheckerex.h"
#endif

#include <DApplication>

#include <QCommandLineParser>
#include <QDebug>
#include <QTranslator>

#include "org_deepin_dde_display1.h"

DWIDGET_USE_NAMESPACE
using namespace org::deepin::dde;

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setQuitOnLastWindowClosed(true);

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" +
                    QLocale::system().name());
    app.installTranslator(&translator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addPositionalArgument("touchscreen",
                                 "the UUID of touchscreen.",
                                 "[touchscreen]");
    parser.process(app);

    const QStringList &posArguments = parser.positionalArguments();

    if (posArguments.isEmpty()) {
        qDebug() << "empty touchscreen UUID";
        return -1;
    }

    Display1 display("org.deepin.dde.Display1", "/org/deepin/dde/Display1", QDBusConnection::sessionBus());
    // 只有一块屏幕时不显示触摸选择提示框
    if (display.monitors().size() < 2) {
        return -1;
    }

    TouchscreenSetting s(posArguments.first());
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    AccessibilityCheckerEx checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif
    s.show();
    s.moveToCenter();
    return app.exec();
}
