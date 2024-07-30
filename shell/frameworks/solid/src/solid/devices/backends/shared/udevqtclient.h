/*
    SPDX-FileCopyrightText: 2009 Benjamin K. Stuhl <bks24@cornell.edu>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UDEVQTCLIENT_H
#define UDEVQTCLIENT_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "udevqtdevice.h"

namespace UdevQt
{
class ClientPrivate;
class Client : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList watchedSubsystems READ watchedSubsystems WRITE setWatchedSubsystems)

public:
    Client(QObject *parent = nullptr);
    Client(const QStringList &subsystemList, QObject *parent = nullptr);
    ~Client() override;

    QStringList watchedSubsystems() const;
    void setWatchedSubsystems(const QStringList &subsystemList);

    DeviceList allDevices();
    DeviceList devicesByProperty(const QString &property, const QVariant &value);
    DeviceList devicesBySubsystem(const QString &subsystem);
    /**
     * Returns a list of devices matching any of the given subsystems AND any of the properties.
     *
     * (subsystem1 || subsystem2 || ...) && (property1 || property2 || ...)
     */
    DeviceList devicesBySubsystemsAndProperties(const QStringList &subsystems, const QVariantMap &properties);
    Device deviceByDeviceFile(const QString &deviceFile);
    Device deviceBySysfsPath(const QString &sysfsPath);
    Device deviceBySubsystemAndName(const QString &subsystem, const QString &name);

Q_SIGNALS:
    void deviceAdded(const UdevQt::Device &dev);
    void deviceRemoved(const UdevQt::Device &dev);
    void deviceChanged(const UdevQt::Device &dev);
    void deviceOnlined(const UdevQt::Device &dev);
    void deviceOfflined(const UdevQt::Device &dev);
    void deviceBound(const UdevQt::Device &dev);
    void deviceUnbound(const UdevQt::Device &dev);

private:
    friend class ClientPrivate;
    ClientPrivate *d;
};

}

#endif
