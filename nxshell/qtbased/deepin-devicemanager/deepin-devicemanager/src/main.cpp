// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Page/MainWindow.h"
#include "DTitlebar"
#include "DApplicationSettings"
#include "environments.h"
#include "DebugTimeManager.h"
#include "SingleDeviceManager.h"
#include "DDLog.h"
#include <DApplication>
#include <DWidgetUtil>
#include <DLog>

#include <QDBusConnection>
#include <QDBusInterface>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <DLog>
#include "LogConfigread.h"

#include <polkit-qt5-1/PolkitQt1/Authority>

using namespace DDLog;

DWIDGET_USE_NAMESPACE

#ifdef OS_BUILD_V23
const QString SERVICE_NAME = "org.deepin.dde.Notification1";
const QString DEVICE_SERVICE_PATH = "/org/deepin/dde/Notification1";
const QString DEVICE_SERVICE_INTERFACE = "org.deepin.dde.Notification1";
#else
const QString SERVICE_NAME = "com.deepin.dde.Notification";
const QString DEVICE_SERVICE_PATH = "/com/deepin/dde/Notification";
const QString DEVICE_SERVICE_INTERFACE = "com.deepin.dde.Notification";
#endif

DCORE_USE_NAMESPACE
using namespace PolkitQt1;

void notify(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    MLogger();

    // /usr/bin/devicemanager notify
    if (argc > 2 && QString(argv[1]).contains("notify")) {
        notify(argc, argv);
        return -1;
    }
    #if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
        Dtk::Core::DLogManager::registerJournalAppender();
    #else
        Dtk::Core::DLogManager::registerFileAppender();
    #endif
    #ifdef QT_DEBUG
        Dtk::Core::DLogManager::registerConsoleAppender();
    #endif

    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }

    PERF_PRINT_BEGIN("POINT-01", "");

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    SingleDeviceManager app(argc, argv);
    app.setAutoActivateWindows(true);
    DBusDriverInterface::getInstance();

    Authority::Result result = Authority::instance()->checkAuthorizationSync("com.deepin.deepin-devicemanager.checkAuthentication",
                                                                             UnixProcessSubject(getpid()),
                                                                             Authority::AllowUserInteraction);
    if (result != Authority::Yes)
        return 0;

    // 保证进程唯一性
    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");
    /*if (DGuiApplicationHelper::instance()->setSingleInstance("deepin-devicemanager",
                                                             DGuiApplicationHelper::UserScope))*/ {
        app.loadTranslator();
        app.setOrganizationName("deepin");
        app.setApplicationName("deepin-devicemanager");
        app.setApplicationDisplayName(QObject::tr("Device Manager"));
        app.setApplicationVersion(VERSION);
        app.setProductName(QObject::tr("Device Manager"));
        app.setApplicationDescription(QObject::tr("Device Manager is a handy tool for viewing hardware information and managing the devices.") + "\n");
        const QString acknowledgementLink = "https://www.deepin.org/original/device-manager/";
        app.setApplicationAcknowledgementPage(acknowledgementLink);
        DApplicationSettings settinAgs;
        Dtk::Core::DLogManager::registerFileAppender();

        QIcon appIcon = QIcon::fromTheme("deepin-devicemanager");

        if (false == appIcon.isNull()) {
            app.setProductIcon(appIcon);
            app.setWindowIcon(appIcon);
        }

        QDBusConnection dbus = QDBusConnection::sessionBus();
        if (dbus.registerService("com.deepin.DeviceManagerNotify")) {
            dbus.registerObject("/com/deepin/DeviceManagerNotify", &app, QDBusConnection::ExportScriptableSlots);
            app.parseCmdLine();
            app.activateWindow();
            return app.exec();
        } else {
            QCommandLineParser parser;
            parser.process(app);

            QVariant var = "";
            if (parser.positionalArguments().size() > 0) {
                var = parser.positionalArguments().at(0);
            }
            qCInfo(appLog) << var;
            QDBusInterface notification("com.deepin.DeviceManagerNotify", "/com/deepin/DeviceManagerNotify", "com.deepin.DeviceManagerNotify", QDBusConnection::sessionBus());
            QDBusMessage msg = notification.call(QDBus::AutoDetect, "startDeviceManager", var);
            return 0;
        }
    }
}

void notify(int argc, char *argv[])
{
    // 1. 连接到dbus
    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. 加载翻译文件
    QString body = QObject::tr("New drivers available! Install or update them now.");
    QString view = QObject::tr("View");
    QString l = QString(argv[2]);
    if ("zh_CN" == l) {
        body = QString("您有驱动可进行安装/更新");
        view = QString("查 看");
    } else if ("zh_HK" == l) {
        body = QString("您有驅動可進行安裝/更新");
        view = QString("查 看");
    } else if ("zh_TW" == l) {
        body = QString("您有驅動可進行安裝/更新");
        view = QString("查 看");
    }

    // 3. create interface
    QDBusInterface *mp_Iface = new QDBusInterface(SERVICE_NAME, DEVICE_SERVICE_PATH, DEVICE_SERVICE_INTERFACE, QDBusConnection::sessionBus());

    QString appname("deepin-devicemanager");
    uint replaces_id = 0;
    QString appicon("deepin-devicemanager");
    QString title = "";

    QStringList actionlist;
    actionlist << "view" << view;
    QVariantMap hints;
    hints.insert(QString("x-deepin-action-view"),
                 QVariant(QString("/usr/bin/deepin-devicemanager,driver")));  //实现查看2按钮点击打开控制中心账户界面)
    int timeout = 3000;

    int count = 0;
    while (count < 10) {
        QDBusReply<uint32_t> reply  = mp_Iface->call("Notify", appname, replaces_id, appicon, title, body, actionlist, hints, timeout);
        if (reply.isValid()) {
            return;
        }
        count++;
    }
}
