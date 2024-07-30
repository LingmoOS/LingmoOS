/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QMap>

#include <KDEDModule>

#include <BluezQt/Manager>
#include <BluezQt/ObexManager>

typedef QMap<QString, QString> DeviceInfo;
typedef QMap<QString, DeviceInfo> QMapDeviceInfo;

class Q_DECL_EXPORT BlueDevilDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.BlueDevil")

public:
    BlueDevilDaemon(QObject *parent, const QList<QVariant> &);
    ~BlueDevilDaemon() override;

    /**
     * Returns whether the daemon is in online mode (eg. Bluez services are
     * running and we have usable adapter)
     */
    Q_SCRIPTABLE bool isOnline();

    /**
     * Returns QMap<Address, DeviceInfo> with all known devices
     */
    Q_SCRIPTABLE QMapDeviceInfo allDevices();

    /**
     * Returns DeviceInfo for one device.
     */
    Q_SCRIPTABLE DeviceInfo device(const QString &address);

    /**
     * Starts discovery for timeout miliseconds (0 = forever)
     */
    Q_SCRIPTABLE void startDiscovering(quint32 timeout);

    /**
     * Stops discovery (if it was previously started)
     */
    Q_SCRIPTABLE void stopDiscovering();

    BluezQt::Manager *manager() const;
    BluezQt::ObexManager *obexManager() const;

private Q_SLOTS:
    void initJobResult(BluezQt::InitManagerJob *job);
    void initObexJobResult(BluezQt::InitObexManagerJob *job);

    void operationalChanged(bool operational);
    void obexOperationalChanged(bool operational);

    void agentRegisted(BluezQt::PendingCall *call);
    void agentRequestedDefault(BluezQt::PendingCall *call);
    void obexAgentRegistered(BluezQt::PendingCall *call);

private:
    DeviceInfo deviceToInfo(BluezQt::DevicePtr device) const;

private:
    struct Private;
    Private *const d;
};
