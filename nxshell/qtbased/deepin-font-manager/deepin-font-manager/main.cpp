// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "globaldef.h"
#include "performancemonitor.h"
#include "utils.h"
#include "views/dfontmgrmainwindow.h"
#include "singlefontapplication.h"
#include "environments.h"

#include <DApplication>
#include <DLog>
#include <DWidgetUtil>
#include <DApplicationSettings>

#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

/*************************************************************************
 <Function>      main
 <Description>   工程主函数入口
 <Author>
 <Input>
    <param1>     argc            Description:标准参数
    <param2>     argv[]          Description:标准参数
 <Return>        int             Description:返回函数处理结果
 <Note>          null
*************************************************************************/
int main(int argc, char *argv[])
{
    PerformanceMonitor::initializeAppStart();

    // 依赖DTK的程序，如果要在root下或者非deepin/uos环境下运行不会发生异常，就需要加上该环境变量
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1); //setenv改变或添加一个环境变量
    }

    // load dtk xcb plugin.
    //DEPRECATED API and remove it
    //DApplication::loadDXcbPlugin();
    // init Dtk application's attrubites.
    SingleFontApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setApplicationName("deepin-font-manager");
    app.setOrganizationName("deepin");
    app.setApplicationVersion(VERSION);
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-font-installer/");
    app.setProductIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));
    app.setProductName(DApplication::translate("Main", "Font Manager"));
    app.setApplicationDescription(DApplication::translate("Main", "Font Manager helps users install and manage fonts."));
    app.setApplicationDisplayName(DApplication::translate("Main", "Font Manager"));

    DApplicationSettings savetheme;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();


//    qDebug() << DTK_VERSION << endl;
    /* 使用DBus实现单例模式 UT000591 */
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if (dbus.registerService("com.deepin.FontManager")) {
        dbus.registerObject("/com/deepin/FontManager", &app, QDBusConnection::ExportScriptableSlots);
        app.parseCmdLine();
        app.activateWindow();
        return app.exec();
    } else {
        app.parseCmdLine(true);
        return 0;
    }
}
