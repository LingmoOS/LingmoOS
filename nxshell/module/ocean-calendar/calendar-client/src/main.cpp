// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "environments.h"
#include "calendarmainwindow.h"
#include "exportedinterface.h"
#include "configsettings.h"
#include "accessible/accessible.h"
#include "tabletconfig.h"
#include "schedulemanager.h"
#include "commondef.h"
#include <LogManager.h>

#include <DApplication>
#include <DLog>
#include <DGuiApplicationHelper>
#include <DApplicationSettings>

#include <QDBusConnection>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    //在root下或者非lingmo/uos环境下运行不会发生异常，需要加上XDG_CURRENT_DESKTOP=Lingmo环境变量；
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("lingmo")) {
        setenv("XDG_CURRENT_DESKTOP", "Lingmo", 1);
    }
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    //适配lingmo-turbo启动加速
    DApplication *app = nullptr;
#if(DTK_VERSION < DTK_VERSION_CHECK(5,4,0,0))
    app = new DApplication(argc, argv);
#else
    app = DApplication::globalApplication(argc, argv);
#endif

    app->setAutoActivateWindows(true);
    //如果dtk版本为5.2.0.1以上则使用新的dtk接口
#if (DTK_VERSION > DTK_VERSION_CHECK(5, 2, 0, 1))
    //设置为-1将永久等待
    DGuiApplicationHelper::setSingleInstanceInterval(-1);
#endif

    if (DGuiApplicationHelper::setSingleInstance(app->applicationName(), DGuiApplicationHelper::UserScope)) {
        //准备数据
        gAccountManager->resetAccount();

        app->setOrganizationName("lingmo");
        app->setApplicationName("ocean-calendar");
        app->loadTranslator();
#ifdef QT_DEBUG
        // 在开发调试时使用项目内的翻译文件
        auto tf = "../translations/ocean-calendar_zh_CN";
        qCDebug(ClientLogger) << "load translate" << tf;
        QTranslator translator;
        translator.load(tf);
        app->installTranslator(&translator);
#endif
        app->setApplicationVersion(VERSION);
        // meta information that necessary to create the about dialog.
        app->setProductName(QApplication::translate("CalendarWindow", "Calendar"));
        QIcon t_icon = QIcon::fromTheme("ocean-calendar");
        app->setProductIcon(t_icon);
        app->setApplicationDescription(QApplication::translate("CalendarWindow", "Calendar is a tool to view dates, and also a smart daily planner to schedule all things in life. "));
        app->setApplicationAcknowledgementPage("https://www.lingmo.org/acknowledgments/ocean-calendar");
        //命令行参数
        QCommandLineParser _commandLine; //建立命令行解析
        _commandLine.addHelpOption(); //增加-h/-help解析命令
        _commandLine.addVersionOption(); //增加-v 解析命令
        _commandLine.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
        _commandLine.process(*app);

        app->setAutoActivateWindows(true);

        bool isOk = false;
        int viewtype = CConfigSettings::getInstance()->value("base.view").toInt(&isOk);
        if (!isOk)
            viewtype = 2;
        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();
        DLogManager::registerJournalAppender();
        //为了与老版本配置兼容
        Calendarmainwindow ww(viewtype - 1);
        ExportedInterface einterface(&ww);
        einterface.registerAction("CREATE", "create a new schedule");
        einterface.registerAction("VIEW", "check a date on calendar");
        einterface.registerAction("QUERY", "find a schedule information");
        einterface.registerAction("CANCEL", "cancel a schedule");
        QDBusConnection dbus = QDBusConnection::sessionBus();
        //如果注册失败打印出失败信息
        if (!dbus.registerService("com.lingmo.Calendar")) {
            qCWarning(ClientLogger) << "registerService Error:" << dbus.lastError();
        }
        if (!dbus.registerObject("/com/lingmo/Calendar", &ww)) {
            qCWarning(ClientLogger) << "registerObject Error:" << dbus.lastError();
        }
        ww.slotTheme(DGuiApplicationHelper::instance()->themeType());
        ww.show();

        qCDebug(ClientLogger) << "ocean-calendar-service start";
        return app->exec();
    }
    return 0;
}
