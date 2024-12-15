// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "environments.h"
#include "dbusservice/grandsearchinterface.h"
#include "global/grandsearch_global.h"

#include <DLog>
#include <DApplication>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusError>
#include <QDebug>
#include <QThread>
#include <QDir>

#include <unistd.h>
#include <signal.h>

GRANDSEARCH_USE_NAMESPACE
DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

static GrandSearchInterface *instance = nullptr;

#define DGSLIB_VERSION_MAJOR 1
#define DGSLIB_VERSION_MINOR 0
#define DGSLIB_VERSION_PATCH 0

#ifdef __cplusplus
extern "C" {
#endif

int startGrandSearchDaemon(int argc, char *argv[])
{
    if (instance) {
        qWarning() << "repeat start: the grand search daemon is running.";
        return 0;
    }

    if (qApp == nullptr) {
        qCritical() << "the Qt Appliaction has not been initialized.";
        return 1;
    }

    if (QThread::currentThread() != qApp->thread()) {
        qCritical() << "startGrandSearchDaemon must be called in main thread at Qt.";
        return 2;
    }

    //服务接口
    GrandSearchInterface *interface = new GrandSearchInterface();

    //注册DBus服务
    {
        QDBusConnection session = interface->qDbusConnection();
        if (!session.registerService(GrandSearchServiceName)) {
            qCritical() << "registerService Failed, maybe service exist" << GrandSearchServiceName << QDBusError::errorString(session.lastError().type());
            delete interface;
            return 3;
        }

        if (!session.registerObject(GrandSearchServicePath, interface,
                                 QDBusConnection::ExportScriptableSlots |
                                 QDBusConnection::ExportScriptableSignals |
                                 QDBusConnection::ExportScriptableProperties)) {
            qCritical() << "registerObject Failed" << GrandSearchServicePath << QDBusError::errorString(session.lastError().type());
            delete interface;
            return 4;
        }
    }

    //初始化
    if (!interface->init()) {
        qCritical() << "failed to initialize grand search deamon." << VERSION;
        return -1;
    }

    instance = interface;
    qInfo() << "grand search deamon is started." << VERSION;

    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName("ocean-grand-search");
    qApp->loadTranslator();
    qApp->setApplicationName(appName);

    return 0;
}

int stopGrandSearchDaemon()
{
    if (instance) {
        qInfo() << "stop grand search deamon";
        delete instance;
        instance = nullptr;
    }
    return 0;
}

const char *grandSearchDaemonAppVersion()
{
    return VERSION;
}

int grandSearchDaemonLibVersion()
{
    return QT_VERSION_CHECK(DGSLIB_VERSION_MAJOR, DGSLIB_VERSION_MINOR, DGSLIB_VERSION_PATCH);
}

static void appExitHandler(int sig) {
    qInfo() << "signal" << sig << "exit.";

    //释放资源，退出插件子进程。
    qApp->quit();
}

int main(int argc, char *argv[])
{
    //安全退出
    signal(SIGINT, appExitHandler);
    signal(SIGQUIT, appExitHandler);
    signal(SIGTERM, appExitHandler);

    DApplication app(argc, argv);

    {
        QString dateTime = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
        //设置应用信息
        app.setOrganizationName("lingmo");
        app.setApplicationName("ocean-grand-search-daemon");
        app.setApplicationVersion(VERSION);

        // 设置终端和文件记录日志
        const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
        DLogManager::setLogFormat(logFormat);
        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();
        qInfo() << dateTime << "starting " << app.applicationName() << app.applicationVersion() << getpid();
    }

    //初始化
    int ret = startGrandSearchDaemon(argc, argv);
    if (ret != 0) {
        qCritical() << "initialization failed.";
        return ret;
    }
    ret = app.exec();
    stopGrandSearchDaemon();

    return ret;
}

#ifdef __cplusplus
}
#endif
