// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgetsserver.h"
#include "accessible/accessible.h"

#include <QCommandLineOption>
#include <QCommandLineParser>

#include <DWidgetUtil>
#include <DApplication>
#include <DLog>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

WIDGETS_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // no inactive color for the application, and it need to be set before DApplication constructor.
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    DApplication a(argc, argv);
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-widgets");

    // fixed font size(qt5integration/pull/199)
    QFont f = a.font();
    f.setPointSize(11);
    a.setFont(f);

    QCommandLineOption showOption(QStringList() << "s"
                                                << "show",
                                  "Show dde-widgets(hide for default).");
    QCommandLineParser parser;
    parser.setApplicationDescription("dde-widgets is the desktop widgets service/implementation for DDE.\n"
                                     "We can show it by call `qdbus org.deepin.dde.Widgets1 /org/deepin/dde/Widgets1 org.deepin.dde.Widgets1.Show`");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(showOption);
    parser.process(a);
    const bool isShow = parser.isSet(showOption);

    // avoid to being quit when dialog is closed with it's parent invisible.
    a.setQuitOnLastWindowClosed(false);
    // enable accessible
    QAccessible::installFactory(accessibleFactory);

    // TODO it's temporay in dbug.
//    Dtk::Core::DLogManager::registerConsoleAppender();
//    Dtk::Core::DLogManager::registerFileAppender();
    qInfo() << "Log path is:" << Dtk::Core::DLogManager::getlogFilePath();

    a.loadTranslator();

    WidgetsServer server;
    if (server.registerService()) {
        qInfo(dwLog()) << "start dde-widgets successfully.";
    } else {
        qInfo(dwLog()) << "start dde-widgets failed.";
        return 1;
    }
    server.start();
    if (isShow)
        server.Show();

    return a.exec();
}
