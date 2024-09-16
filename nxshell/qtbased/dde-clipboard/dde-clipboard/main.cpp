// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "constants.h"
#include "clipboard_adaptor.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <unistd.h>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::ColorCompositing, true);

    DApplication *app = DApplication::globalApplication(argc, argv);
    DApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    app->setOrganizationName("deepin");
    app->setApplicationName("dde-clipboard");
    app->setApplicationDisplayName("DDE Clipboard");
    app->setApplicationVersion("1.0");

    QCommandLineOption alwaysShowOption("always-show" , "show and will never hide");

    QCommandLineParser parser;
 
    parser.setApplicationDescription("DDE Clipboard");
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(alwaysShowOption);

    parser.process(*app);

    app->loadTranslator();

    if (!DGuiApplicationHelper::setSingleInstance(QString("dde-clipboard_%1").arg(getuid()))) {
        qDebug() << "set single instance failed!";
        return -1;
    }

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();
    DLogManager::registerJournalAppender();

    QDBusConnection connection = QDBusConnection::sessionBus();

    MainWindow w;

    bool alwaysShow = parser.isSet(alwaysShowOption);
    if (alwaysShow) {
        w.setAlwaysShow(true);
    }

    QDBusInterface interface("org.deepin.dde.ClipboardLoader1", "/org/deepin/dde/ClipboardLoader1",
                                 "org.deepin.dde.ClipboardLoader1",
                                 QDBusConnection::sessionBus());

    ClipboardAdaptor adapt(&w);
    if (!connection.registerService(DBusClipBoardService)) {
        qDebug() << "error:" << connection.lastError().message();
        exit(-1);
    }

    connection.registerObject(DBusClipBoardPath, &w);

    if (alwaysShow) {
        w.showAni();
    }

    return app->exec();
}
