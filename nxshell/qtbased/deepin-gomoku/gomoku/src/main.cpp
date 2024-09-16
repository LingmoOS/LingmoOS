// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gomokumainwindow.h>
#include "environments.h"
#include "gomokuapplication.h"

#include <DApplication>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DApplicationSettings>
#include <DLog>

#include <QAccessible>
#include <QCommandLineParser>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{

//    PERF_PRINT_BEGIN("POINT-01", "");
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    Gomokuapplication *app = nullptr;
    app = new Gomokuapplication(argc, argv);

    app->setAutoActivateWindows(true);
    //如果dtk版本为5.2.0.1以上则使用新的dtk接口
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 2, 0, 1))
    //设置为-1将永久等待
    DGuiApplicationHelper::setSingleInstanceInterval(-1);
#endif

    if (DGuiApplicationHelper::setSingleInstance(app->applicationName(), DGuiApplicationHelper::UserScope)) {
//        QAccessible::installFactory(accessibleFactory);
        app->setOrganizationName("deepin");
        app->setApplicationName("deepin-gomoku");
        app->loadTranslator();
        app->setApplicationVersion(VERSION);

        QIcon t_icon = QIcon::fromTheme("com.deepin.gomoku");
        app->setProductIcon(t_icon);
        app->setProductName(QApplication::translate("GomokuWindow", "Gomoku"));
        app->setApplicationDisplayName(QApplication::translate("GomokuWindow", "Gomoku"));
        app->setApplicationDescription(QApplication::translate("GomokuWindow", "Gomoku is a small chess game for two players."));

        //命令行参数
        QCommandLineParser _commandLine; //建立命令行解析
        _commandLine.addHelpOption(); //增加-h/-help解析命令
        _commandLine.addVersionOption(); //增加-v 解析命令
        _commandLine.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
        _commandLine.process(*app);

        app->setAutoActivateWindows(true);
        //log输出到控制台
        DLogManager::registerConsoleAppender();
        //log输出到日志文件
        DLogManager::registerFileAppender();
        //保存主题
        DApplicationSettings applicationset(app);
        //加载系统语言
        Globaltool::instacne()->loadSystemLanguage();
        GomokuMainWindow ww;
        ww.show();

        return app->exec();
    }

    return 0;
}
