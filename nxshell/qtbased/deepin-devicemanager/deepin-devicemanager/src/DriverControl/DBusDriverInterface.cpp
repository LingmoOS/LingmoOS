// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DBusDriverInterface.h"
#include <unistd.h>

#include <QApplication>

// 以下这个问题可以避免单例的内存泄露问题
std::atomic<DBusDriverInterface *> DBusDriverInterface::s_Instance;
std::mutex DBusDriverInterface::m_mutex;

const QString SERVICE_NAME = "org.deepin.DeviceControl";
const QString DRIVER_SERVICE_PATH = "/org/deepin/DeviceControl";
const QString DRIVER_INTERFACE = "org.deepin.DeviceControl";

void DBusDriverInterface::uninstallDriver(const QString &driver)
{
    QDBusPendingCall async = mp_Iface->asyncCall("unInstallDriver", driver);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
                     this, SLOT(slotCallFinished(QDBusPendingCallWatcher *)));
}

void DBusDriverInterface::uninstallPrinter(const QString &vendor, const QString &model)
{
    //Sets the timeout in milliseconds for all future DBus calls to timeout. -1 means the default DBus timeout (usually 25 seconds).
    //超时默认是25s，打印卸载超时大概30s
    mp_Iface->setTimeout(1000 * 1000);
    QDBusPendingCall async = mp_Iface->asyncCall("unInstallPrinter", vendor, model);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
                     this, SLOT(slotCallFinished(QDBusPendingCallWatcher *)));
}

void DBusDriverInterface::installDriver(const QString &driver)
{
    QDBusPendingCall async = mp_Iface->asyncCall("installDriver", driver);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
                     this, SLOT(slotCallFinished(QDBusPendingCallWatcher *)));
}

void DBusDriverInterface::installDriver(const QString &driverName, const QString &version)
{
    QDBusPendingCall async = mp_Iface->asyncCall("installDriver", driverName, version);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(async, this);
    QObject::connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)),
                     this, SLOT(slotCallFinished(QDBusPendingCallWatcher *)));
}

void DBusDriverInterface::undoInstallDriver()
{
    mp_Iface->call("undoInstallDriver");
}

bool DBusDriverInterface::isDriverPackage(const QString &path)
{
    mp_Iface->setTimeout(1000 * 1000);
    QDBusReply<bool> reply = mp_Iface->call("isDriverPackage", path);
    if (reply.isValid())
        return reply.value();
    return false;
}

bool DBusDriverInterface::isArchMatched(const QString &path)
{
    QDBusReply<bool> reply = mp_Iface->call("isArchMatched", path);
    if (reply.isValid())
        return reply.value();
    return false;
}

bool DBusDriverInterface::isDebValid(const QString &path)
{
    QDBusReply<bool> reply = mp_Iface->call("isDebValid", path);
    if (reply.isValid())
        return reply.value();
    return false;
}

bool DBusDriverInterface::backupDeb(const QString &debpath)
{
    QDBusReply<bool> reply = mp_Iface->call("backupDeb", debpath);

    return reply.value();
}

bool DBusDriverInterface::delDeb(const QString &debname)
{
    QDBusReply<bool> reply = mp_Iface->call("delDeb",debname);

    return reply.value();
}

bool DBusDriverInterface::aptUpdate()
{
    QDBusReply<bool> reply = mp_Iface->call("aptUpdate");

    return reply.value();
}

DBusDriverInterface::DBusDriverInterface(QObject *parent)
    : QObject(parent)
    , mp_Iface(nullptr)
{
    init();
}

DBusDriverInterface::~DBusDriverInterface()
{

}

void DBusDriverInterface::slotProcessChange(qint32 value, QString detail)
{
    emit processChange(value, detail);
}

void DBusDriverInterface::slotProcessEnd(bool success, QString msg)
{
    if (success) {
        emit processChange(100, "");
        usleep(300000);
    }
    emit processEnd(success, msg);
}

void DBusDriverInterface::slotCallFinished(QDBusPendingCallWatcher *watcher)
{
    QDBusPendingReply<bool> reply = *watcher;
    watcher->deleteLater();
}

void DBusDriverInterface::slotDownloadProgressChanged(QStringList msg)
{
    emit downloadProgressChanged(msg);
}

void DBusDriverInterface::slotDownloadFinished()
{
    emit downloadFinished();
}

void DBusDriverInterface::slotInstallProgressFinished(bool bsuccess, int err)
{
    emit installProgressFinished(bsuccess, err);
}

void DBusDriverInterface::slotInstallProgressChanged(qint32 progress)
{
    emit installProgressChanged(progress);
}

void DBusDriverInterface::init()
{
    // 1. 连接到dbus
    if (!QDBusConnection::systemBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus session bus./n"
                "To start it, run:/n"
                "/teval `dbus-launch --auto-syntax`/n");
    }

    // 2. create interface
    mp_Iface = new QDBusInterface(SERVICE_NAME, DRIVER_SERVICE_PATH, DRIVER_INTERFACE, QDBusConnection::systemBus());

    if (mp_Iface->isValid()) {
        connect(mp_Iface, SIGNAL(sigProgressDetail(qint32, QString)), this, SLOT(slotProcessChange(qint32, QString)));
        connect(mp_Iface, SIGNAL(sigFinished(bool, QString)), this, SLOT(slotProcessEnd(bool, QString)));

        connect(mp_Iface, SIGNAL(sigDownloadProgressChanged(QStringList)), this, SLOT(slotDownloadProgressChanged(QStringList)));
        connect(mp_Iface, SIGNAL(sigDownloadFinished()), this, SLOT(slotDownloadFinished()));
        connect(mp_Iface, SIGNAL(sigInstallProgressChanged(qint32)), this, SLOT(slotInstallProgressChanged(qint32)));
        connect(mp_Iface, SIGNAL(sigInstallProgressFinished(bool, int)), this, SLOT(slotInstallProgressFinished(bool, int)));
        connect(mp_Iface, SIGNAL(sigFinished(bool, QString)), this, SIGNAL(installFinished(bool, QString)));
        connect(mp_Iface, SIGNAL(sigProgressDetail(int, QString)), this, SIGNAL(installProgressDetail(int, QString)));
        connect(mp_Iface, SIGNAL(sigBackupProgressFinished(bool)), this, SIGNAL(backupProgressFinished(bool)));

    }
}
