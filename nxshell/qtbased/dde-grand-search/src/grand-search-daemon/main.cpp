// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "environments.h"
#include "daemonlibrary.h"

#include <DLog>
#include <DApplication>

#include <QDebug>
#include <QDir>

#include <unistd.h>
#include <signal.h>

GRANDSEARCH_USE_NAMESPACE

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

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
        app.setOrganizationName("deepin");
        app.setApplicationName("dde-grand-search-daemon");
        app.setApplicationVersion(VERSION);

        // 设置终端和文件记录日志
        const QString logFormat = "%{time}{yyyyMMdd.HH:mm:ss.zzz}[%{type:1}][%{function:-35} %{line:-4} %{threadid} ] %{message}\n";
        DLogManager::setLogFormat(logFormat);
        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();
        qInfo() << dateTime << "starting " << app.applicationName() << app.applicationVersion() << getpid();
    }

    QString libPath;
    {
        //默认路径
    #ifdef QT_DEBUG
        char path[PATH_MAX] = {0};
        const char *defaultPath = realpath("../libgrand-search-daemon", path);
    #else
        auto defaultPath = DAEMON_LIB_BASE_DIR;
    #endif
        static_assert(std::is_same<decltype(defaultPath), const char *>::value, "DAEMON_LIB_BASE_DIR is not a string.");

        QDir dir(defaultPath);
        libPath = dir.absoluteFilePath("libdde-grand-search-daemon.so");
        qInfo() << "daemon lib path:" << libPath;
    }

    DaemonLibrary lib(libPath);
    if (!lib.load())
        return -1;

    //初始化
    qInfo() << "lib start " << lib.version();
    int ret = lib.start(argc, argv);
    if (ret != 0) {
        qCritical() << "initialization failed.";
        return ret;
    }
    ret = app.exec();
    lib.stop();

    return ret;
}
