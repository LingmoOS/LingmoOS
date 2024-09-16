// Copyright (C) 2011 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "accessible.h"
#include "common/common.h"
#include "application.h"
#include "settings.h"
#include "gui/main_window.h"
#include "common/perf.h"
#include "dbus/dbus_object.h"
#include "dbus/dbusalarmnotify.h"
#include "3rdparty/dmidecode/dmidecode.h"

#include <DApplication>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>
#include <DMainWindow>
#include <DWidgetUtil>
#include <DLog>

#include <QApplication>
#include <QDateTime>
#include <QAccessible>
#include "logger.h"
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

using namespace common::init;

int main(int argc, char *argv[])
{
    MLogger();   // 日志处理要放在app之前，否则QApplication
            // 内部可能进行了日志打印，导致环境变量设置不生效
// 为了兼容性
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    Dtk::Core::DLogManager::registerJournalAppender();
#else
    Dtk::Core::DLogManager::registerFileAppender();
#endif
#ifdef QT_DEBUG
    Dtk::Core::DLogManager::registerConsoleAppender();
#endif

    //Judge if Wayland
    WaylandSearchCentered();
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    Application::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    Application app(argc, argv);
    QCommandLineParser parser;
    parser.process(app);
    QStringList allArguments = parser.positionalArguments();
    if (allArguments.size() == 3 && allArguments.first().compare("alarm", Qt::CaseInsensitive) == 0) {
        app.loadTranslator();
        DBusAlarmNotify::getInstance().showAlarmNotify(allArguments);
        return 0;
    }

    //=======通知已经打开的进程
    if (!DBusObject::getInstance().registerOrNotify())
        return 0;

    //获取dmidecode中CPU频率信息
    char *const cmd[] = { "dmidecode", "-t", "4" };
    get_cpuinfo_from_dmi(3, cmd);
    PERF_PRINT_BEGIN("POINT-01", "");

    app.setAutoActivateWindows(true);
    //设置单例
    DGuiApplicationHelper::setSingleInstanceInterval(-1);
    if (DGuiApplicationHelper::setSingleInstance(QString("deepin-system-monitor"),
                                                 DGuiApplicationHelper::UserScope)) {
        app.loadTranslator();

        const QString descriptionText = DApplication::translate(
                "App.About",
                "System Monitor is a tool to monitor realtime system load, "
                "view and control processes and services running on your system.");

        const QString acknowledgementLink =
                "https://www.deepin.org/acknowledgments/deepin-system-monitor#thanks";

        app.setOrganizationName("deepin");
        app.setApplicationName("deepin-system-monitor");
        app.setApplicationDisplayName(DApplication::translate("App.About", "System Monitor"));
        app.setApplicationVersion(VERSION);

        app.setProductIcon(QIcon::fromTheme("deepin-system-monitor"));
        app.setProductName(DApplication::translate("App.About", "System Monitor"));
        app.setApplicationDescription(descriptionText);
        app.setApplicationAcknowledgementPage(acknowledgementLink);

        //app.setWindowIcon(QIcon::fromTheme("deepin-system-monitor")); //耗时40ms

        QAccessible::installFactory(accessibleFactory);

        DApplicationSettings appSettings;

        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();

        MainWindow mw;
        gApp->setMainWindow(&mw);
        mw.initDisplay();

        QObject::connect(&app,
                         &DApplication::newInstanceStarted,
                         &mw,
                         &MainWindow::activateWindow);

        Dtk::Widget::moveToCenter(&mw);
        mw.show();

        return app.exec();
    }

    return 0;
}
