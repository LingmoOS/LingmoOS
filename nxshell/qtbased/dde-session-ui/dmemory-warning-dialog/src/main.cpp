// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmemorywarningdialog.h"
#include "dmemorywarningdialogadaptor.h"
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include "../common/accessibilitycheckerex.h"
#endif

#include <DApplication>
#include <DLog>

#include <QDBusConnection>
#include <QTranslator>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *args[])
{
    DApplication dapp(argc, args);
    dapp.setQuitOnLastWindowClosed(false);
    dapp.setAttribute(Qt::AA_UseHighDpiPixmaps);

    if (!dapp.setSingleInstance("dmemory-warning-dialog", DApplication::UserScope))
        return -1;

    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    dapp.installTranslator(&translator);

#ifdef QT_DEBUG
    DLogManager::registerConsoleAppender();
#else
    DLogManager::registerFileAppender();
#endif

    DMemoryWarningDialog dialog;
    DMemoryWarningDialogAdaptor dbusAdaptor(&dialog);
    Q_UNUSED(dbusAdaptor);
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("org.deepin.dde.MemoryWarningDialog1") ||
        !connection.registerObject("/org/deepin/dde/MemoryWarningDialog1", &dialog))
        return -1;

#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    AccessibilityCheckerEx checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();

    dialog.show();
#endif

    return dapp.exec();
}
