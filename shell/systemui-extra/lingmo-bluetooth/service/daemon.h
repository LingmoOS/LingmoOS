/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef DAEMON_H
#define DAEMON_H

#include "bluetoothagent.h"
#include "bluetoothobexagent.h"
#include "config.h"
#include "buriedpointdata.h"

#ifdef BATTERY
#include <KF5/BluezQt/bluezqt/battery.h>
#endif
#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/initobexmanagerjob.h>
#include <KF5/BluezQt/bluezqt/obexobjectpush.h>
#include <KF5/BluezQt/bluezqt/obexsession.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>

#include <QObject>
#include <QCoreApplication>
#include <QString>
#include <QUrl>
#include <QDebug>
#include <QGSettings>
#include <QMap>
#include <QStringList>
#include <QVariant>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QFileDialog>
#include <QFileSystemWatcher>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QThread>
#include <QEventLoop>
#include "lingmo-log4qt.h"
#include "CSingleton.h"

class Daemon : public QObject
{
    Q_OBJECT
public:
    explicit Daemon(QObject *parent = nullptr);
    ~Daemon();

    int start(void);

    int stop(void);

    int unblock_bluetooth(void);

protected:
    int init_bluez(void);

    int init_obex(void);

    int init_sys_monistor(void);
private:
    int __init_bluez_adapter(void);

protected slots:
    //bluez
    void operationalChanged(bool operational);
    void bluetoothOperationalChanged(bool operational);
    void bluetoothBlockedChanged(bool blocked);
    void adapterAdded(BluezQt::AdapterPtr adapter);
    void adapterRemoved(BluezQt::AdapterPtr adapter);
    void usableAdapterChanged(BluezQt::AdapterPtr adapter);
    void allAdaptersRemoved();


    //sleep
    void monitorSleepSlot(bool);
    void activeUserChange(QString);


private:
    void __unblock_sleep(int);

    void wait_for_finish(BluezQt::PendingCall *call);
protected:
    BluezQt::Manager * m_bluez = nullptr;

    friend class SingleTon<Daemon>;
};

typedef SingleTon<Daemon>  DAEMON;

#endif // DAEMON_H
