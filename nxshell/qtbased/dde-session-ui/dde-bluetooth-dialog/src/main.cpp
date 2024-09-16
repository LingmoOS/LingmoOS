// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pincodedialog.h"
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
#include "../common/accessibilitycheckerex.h"
#endif

#include <DApplication>

#include <QDebug>
#include <QTranslator>


DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

const int PingCode = 1;
const int DevicePath = 2;
const int PingTime = 3;

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-bluetooth-dialog");
    QTranslator translator;
    translator.load("/usr/share/dde-session-ui/translations/dde-session-ui_" + QLocale::system().name());
    app.installTranslator(&translator);

    QStringList arguslist = app.arguments();
    if (arguslist.size() < 4) {
        qDebug() << "number of parameters must be greater than 3";
        return -1;
    }
    qDebug() << "PingCode:" << arguslist[PingCode] << " Device Path:" << arguslist[DevicePath] << "Ping Time:" + arguslist[PingTime];

    PinCodeDialog dialog(arguslist[PingCode], arguslist[DevicePath], arguslist[PingTime], true);
#if (defined QT_DEBUG) && (defined CHECK_ACCESSIBLENAME)
    AccessibilityCheckerEx checker;
    checker.setOutputFormat(DAccessibilityChecker::FullFormat);
    checker.start();
#endif
    if (!dialog.isVisible()) {
        dialog.show();
    }
    return app.exec();
}

