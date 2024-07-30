/*
    SPDX-FileCopyrightText: 2009 Benjamin K. Stuhl <bks24@cornell.edu>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QList>
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
    explicit Client(QObject *parent = nullptr);
    explicit Client(const QStringList &subsystemList, QObject *parent = nullptr);
    ~Client() override;

    QStringList watchedSubsystems() const;
    void setWatchedSubsystems(const QStringList &subsystemList);

    DeviceList allDevices();
    DeviceList devicesByProperty(const QString &property, const QVariant &value);
    DeviceList devicesBySubsystem(const QString &subsystem);
    Device deviceByDeviceFile(const QString &deviceFile);
    Device deviceBySysfsPath(const QString &sysfsPath);
    Device deviceBySubsystemAndName(const QString &subsystem, const QString &name);

Q_SIGNALS:
    void deviceAdded(const UdevQt::Device &dev);
    void deviceRemoved(const UdevQt::Device &dev);
    void deviceChanged(const UdevQt::Device &dev);
    void deviceOnlined(const UdevQt::Device &dev);
    void deviceOfflined(const UdevQt::Device &dev);

private:
    friend class ClientPrivate;
    Q_PRIVATE_SLOT(d, void _uq_monitorReadyRead(int fd))
    ClientPrivate *d;
};

}
