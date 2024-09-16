// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "diskmanagerservice.h"
#include "log.h"
#include "watcher.h"

#include <QCoreApplication>
#include <DLog>
#include <QDBusConnection>
#include <QDBusError>
#include <QThread>

const QString DiskManagerServiceName = "com.deepin.diskmanager";
const QString DiskManagerPath = "/com/deepin/diskmanager";

void checkFrontEndQuit(uint frontEndPid)
{
    QString frontEndExe = QString("/proc/%1/exe").arg(frontEndPid);
    QFileInfo info(frontEndExe);

    if (info.symLinkTarget() != "/usr/bin/deepin-diskmanager") {
        QCoreApplication::exit(0);
    }
}

int main(int argc, char *argv[])
{
    //set env otherwise utils excutecmd  excute command failed
    QString PATH = qgetenv("PATH");

    qDebug() << "main start" << PATH;
    if (PATH.isEmpty()) {
        PATH = "/usr/bin";
    }
    PATH += ":/usr/sbin";
    PATH += ":/sbin";
    qputenv("PATH", PATH.toLatin1());

    uint frontEndPid;
    QString frontEndDBusName;
    if (argc < 3)
        return 1;
    else {
        QString frontEndPidString(argv[1]);
        frontEndPid = frontEndPidString.toUInt();
        if (frontEndPid == 0)
            return 1;
        frontEndDBusName = QString(argv[2]);
    }

    QCoreApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-diskmanager-service");

    QDir dirCheck;
    QString LogPath = QString("%1/%2/%3/Log/")
        .arg("/var/log")
        .arg(qApp->organizationName())
        .arg(qApp->applicationName());
    setLogDir(LogPath);
    if (!dirCheck.exists(LogPath))
    {
        dirCheck.mkpath(LogPath);
    }
    //检查日志是否过期
    CheckLogTime();
    //磁盘剩余空间小于阈值，清除早期日志
    CheckFreeDisk();
    //创建新日志
    CreateNewLog();
    qInstallMessageHandler(customLogMessageHandler);
//    Dtk::Core::DLogManager::registerConsoleAppender();
//    Dtk::Core::DLogManager::registerFileAppender();

//    qDebug() << "write log to" << Dtk::Core::DLogManager::getlogFilePath();
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerService(DiskManagerServiceName)) {
        qCritical() << "registerService failed:" << systemBus.lastError();
        exit(0x0001);
    }
    DiskManager::DiskManagerService service(frontEndDBusName);
    qDebug() << "systemBus.registerService success" /*<< Dtk::Core::DLogManager::getlogFilePath()*/;
    if (!systemBus.registerObject(DiskManagerPath,
                                  &service,
                                  QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qCritical() << "registerObject failed:" << systemBus.lastError();
        exit(0x0002);
    }

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [frontEndPid] {
        checkFrontEndQuit(frontEndPid);
    });
    timer.start(1000);

    /*
     * 启动一个线程监测前端是否运行：
     *    1 如果前端没有启动过，则后台保持运行
     *    2 如果前端启动过，又退出了，则后台退出。这可能是因为用户从dock栏强杀了磁盘管理器。
     */
    // DiskManager::Watcher m_watcher;
    // QObject::connect(qApp, &QCoreApplication::aboutToQuit, &m_watcher, &DiskManager::Watcher::exit);
    // m_watcher.start();

    return a.exec();
}
