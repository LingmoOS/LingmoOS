// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbus/zcupsmonitor.h"
#include "dbus/helperinterface.h"
#include "usbprinter/usbthread.h"
#include "zsettings.h"
#include "reviselogger.h"

#include <DApplication>
#include <DLog>
#include <DGuiApplicationHelper>
#include <QDBusConnection>

#include <QDebug>
#include <QProcess>
#include <QIcon>

#include <signal.h>
#include <unistd.h>


DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

void handler(int signo)
{
    //默认终止的自定义信号，此处作为重启通知
    if (signo == SIGUSR1) {
        pid_t pid = getpid();
        QProcess process;
        QString cmd = QString("dde-printer-helper -r %1").arg(pid);
        process.startDetached("bash", QStringList() << "-c" << cmd);
    }
}

int main(int argc, char *argv[])
{
#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    MLogger loggerConf;
#endif

    DApplication a(argc, argv);

    qApp->loadTranslator();
    qApp->setOrganizationName("deepin");
    qApp->setApplicationName("dde-printer-helper");
    qApp->setApplicationVersion(DApplication::buildVersion("1.0"));
    qApp->setProductIcon(QIcon(":/images/dde-printer.svg"));
    qApp->setProductName(QObject::tr("Print Manager"));
    qApp->setApplicationDescription(QObject::tr("Print Manager is a printer management tool, which supports adding and removing printers, managing print jobs and so on."));
    qApp->setApplicationLicense("GPLv3.");
    a.setQuitOnLastWindowClosed(false);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

#if (DTK_VERSION >= DTK_VERSION_CHECK(5, 6, 8, 0))
    DLogManager::registerJournalAppender();
#else
    QString logRules = g_Settings->getLogRules();
    QLoggingCategory::setFilterRules(logRules);
#endif


    if (qApp->arguments().contains("-r")) {
        //重启模式先kill原始进程
        QString originPid = qApp->arguments().at(2).toLocal8Bit();
        QProcess process;
        QString cmd = "kill";
        QStringList args;
        args << "-9" << originPid;
        process.start(cmd, args);

        if (process.waitForFinished()) {
            qInfo() << "kill origin process " << originPid;
        } else {
            qInfo() << "kill origin process failed :" << process.errorString();
        }

    }

    if (!DGuiApplicationHelper::setSingleInstance("dde-printer-helper")) {
        //进程设置单例失败，杀死原始进程，继续设置单例，虽然返回true，但是实际没有生效，所以先杀死原始进程，再设置新进程单例
        qWarning() << "dde-printer-helper is running";
        return -1;
    }
    // 绑定SIGUSR1信号
    if (signal(SIGUSR1, handler) == SIG_ERR) {
        qWarning("Can't set handler for SIGUSR1\n");
        return -2;
    }

    USBThread usbThread;

    CupsMonitor cupsMonitor;
    cupsMonitor.initTranslations();

    HelperInterface helper(&cupsMonitor);
    helper.registerDBus();

    QObject::connect(&usbThread, &USBThread::deviceStatusChanged, &helper, &HelperInterface::deviceStatusChanged);
    QObject::connect(&helper, &HelperInterface::usbDeviceProcess, [&](){
        usbThread.start();
    });

    cupsMonitor.initSubscription();
    cupsMonitor.initWatcher();

    int ret = a.exec();
    helper.unRegisterDBus();
    return ret;
}
