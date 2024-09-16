// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once
#include <qdbuspendingreply.h>
#include <qdebug.h>
#include <qobject.h>
#include <qscopedpointer.h>

class UPowerManagerService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.FakePower")
public:
    explicit UPowerManagerService(QObject *parent = nullptr);
    virtual ~UPowerManagerService();
    // properties
    Q_PROPERTY(bool LidIsClosed READ lidIsClosed);
    Q_PROPERTY(bool LidIsPresent READ lidIsPresent);
    Q_PROPERTY(bool OnBattery READ onBattery);
    Q_PROPERTY(QString DaemonVersion READ daemonVersion);

    inline bool lidIsClosed() const { return false; }

    inline bool lidIsPresent() const { return false; }

    inline bool onBattery() const { return true; }

    inline QString daemonVersion() const { return "YES"; }

public slots:

    Q_SCRIPTABLE void Reset() { m_reset = true; }

    Q_SCRIPTABLE QList<QDBusObjectPath> EnumerateDevices() const;
    Q_SCRIPTABLE QString GetCriticalAction() const;
    Q_SCRIPTABLE QDBusObjectPath GetDisplayDevice() const;

public:
    bool m_reset;

private:
    void registerService();
    void unRegisterService();
};
