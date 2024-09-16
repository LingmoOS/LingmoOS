// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "warningdialog.h"
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include "../common/accessibilitycheckerex.h"
#endif

#include <DApplication>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>

#include <QTranslator>
#include <QDBusConnection>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE

static const QString Service = "org.deepin.dde.WarningDialog1";
static const QString Path = "/org/deepin/dde/WarningDialog1";
static const QString Interface = "org.deepin.dde.WarningDialog1";

int main(int argc, char *argv[])
{
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (QString(qgetenv("XDG_CURRENT_DESKTOP")) != QStringLiteral("DDE")){
        setenv("XDG_CURRENT_DESKTOP", "DDE", 1);
    }

    DApplication a(argc, argv);
    a.setApplicationName("dde-warning-dialog");
    a.setApplicationVersion("1.0");
    a.setQuitOnLastWindowClosed(true);

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    a.installTranslator(&translator);

    DGuiApplicationHelper::setSingleInstanceInterval(-1);
    if (!a.setSingleInstance(a.applicationName(), DApplication::UserScope)) {
        return -1;
    }

    WarningDialog w;
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService(Service);
    connection.registerObject(Path, &w);
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    AccessibilityCheckerEx checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif
    moveToCenter(&w);
    w.show();

    return a.exec();
}
