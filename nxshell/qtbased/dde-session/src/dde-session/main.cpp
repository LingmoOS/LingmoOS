// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QDBusServiceWatcher>
#include <QElapsedTimer>
#include <QTimer>
#include <QDebug>
#include <QtConcurrent>

#include <DLog>

#include <systemd/sd-daemon.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#include "session1adaptor.h"
#include "sessionmanager1adaptor.h"
#include "wmswitcher1adaptor.h"
#include "org_freedesktop_systemd1_Manager.h"
#include "utils/fifo.h"
#include "utils/utils.h"
#include "impl/sessionmanager.h"
#include "impl/wmswitcher.h"
#include "impl/iowait/iowaitwatcher.h"
#include "environmentsmanager.h"
#include "othersmanager.h"

DCORE_USE_NAMESPACE

int startSystemdUnit(org::freedesktop::systemd1::Manager &systemd1, const QString &unitName, const QString &unitType, bool isWait = false)
{
    QDBusPendingReply<QDBusObjectPath> reply = systemd1.StartUnit(unitName, unitType);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "start systemd unit failed:" << unitName;
        return -1;
    }
    qInfo() << "success to start systemd unit:" << unitName << ", job path:" << reply.value().path();

    if (isWait) {
        QEventLoop eLoop;
        QTimer timer;
        QObject::connect(&timer, &QTimer::timeout, &eLoop, &QEventLoop::quit);
        qInfo() << "start systemd unit, wait begin.";
        QMetaObject::Connection conn = QObject::connect(&systemd1, &org::freedesktop::systemd1::Manager::JobRemoved, [reply, &eLoop](uint id, const QDBusObjectPath &job, const QString &unit, const QString &result) mutable {
            qInfo() << "JobRemoved, id:" << id << ", unit:" << unit << ", job:" <<  job.path() << ", result:" << result;
            if (job.path() == reply.value().path()) {
                eLoop.quit();
            }
        });
        timer.setSingleShot(true);
        timer.setInterval(1000 * 30);
        timer.start();
        eLoop.exec();
        timer.stop();

        qInfo() << "start systemd unit, wait end.";
        QObject::disconnect(conn);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("dde-session");

    QCommandLineParser parser;
    parser.setApplicationDescription("dde-session");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption systemd(QStringList{"d", "systemd-service", "wait for systemd services"});
    parser.addOption(systemd);
    parser.process(app);

    if (!parser.isSet(systemd)) {
        DLogManager::registerConsoleAppender();
        DLogManager::registerFileAppender();

        EnvironmentsManager().init();

        QString dmService = "dde-session.target";
        qInfo() << "start dm service:" << dmService;
        org::freedesktop::systemd1::Manager systemdDBus("org.freedesktop.systemd1", "/org/freedesktop/systemd1", QDBusConnection::sessionBus());
        startSystemdUnit(systemdDBus, dmService, "replace");

        QDBusServiceWatcher *watcher = new QDBusServiceWatcher("org.deepin.dde.Session1", QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForUnregistration);
        watcher->connect(watcher, &QDBusServiceWatcher::serviceUnregistered, [&] {
            qInfo() << "dde session exit";
            startSystemdUnit(systemdDBus, "dde-session-exit-task.service", "replace");
            qApp->quit();
        });
        pid_t curPid = getpid();
        QtConcurrent::run([curPid](){
            qInfo()<<"leader pipe thread id: " << QThread::currentThreadId() << ", pid: " << curPid;
            Fifo *fifo = new Fifo;
            fifo->OpenWrite();
            fifo->Write(QString::number(curPid));
        });

        // We started the unit, open <dbus> and sleep forever.
        return app.exec();
    }

    // systemd-service
    auto* session = new Session;
    new Session1Adaptor(session);
    QDBusConnection::sessionBus().registerService("org.deepin.dde.Session1");
    QDBusConnection::sessionBus().registerObject("/org/deepin/dde/Session1", "org.deepin.dde.Session1", session);

    SessionManager::instance()->init();
    new SessionManager1Adaptor(SessionManager::instance());
    QDBusConnection::sessionBus().registerService("org.deepin.dde.SessionManager1");
    QDBusConnection::sessionBus().registerObject("/org/deepin/dde/SessionManager1", "org.deepin.dde.SessionManager1", SessionManager::instance());

    auto *wmSwitcher = new WMSwitcher();
    new WMSwitcher1Adaptor(wmSwitcher);
    QDBusConnection::sessionBus().registerService("org.deepin.dde.WMSwitcher1");
    QDBusConnection::sessionBus().registerObject("/org/deepin/dde/WMSwitcher1", wmSwitcher);

    // TODO 这部分都是一次性运行，可以拆分成不同的oneshot服务
    QtConcurrent::run([] {
        OthersManager().init();
    });

    // cpu iowait状态检测
    if (Utils::SettingValue("com.deepin.dde.startdde", QByteArray(), "iowait-enabled", false).toBool()) {
        QtConcurrent::run([] {
            auto watcher = new IOWaitWatcher;
            watcher->start();
        });
    } else {
        qInfo() << "iowait watcher disabled";
    }

    QtConcurrent::run([&session](){
        qInfo()<< "systemd service pipe thread id: " << QThread::currentThreadId();
        Fifo *fifo = new Fifo;
        fifo->OpenRead();
        qInfo() << "pipe open read finish";
        QString CurSessionPid;
        int len = 0;
        while ((len = fifo->Read(CurSessionPid)) > 0) {
            bool ok;
            int pid = CurSessionPid.toInt(&ok);
            qInfo() << "dde-session pid: " << CurSessionPid;
            if (ok && pid > 0) {
                // TODO session在主线程中创建，在这里直接使用是有问题的
                session->setSessionPid(pid); // TODO: session别直接调用
                session->setSessionPath();
            }
        }
        qInfo() << "pipe read finish, app exit.";
        qApp->quit();
    });
    sd_notify(0, "READY=1");

    return app.exec();
}
