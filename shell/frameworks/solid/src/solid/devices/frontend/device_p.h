/*
    SPDX-FileCopyrightText: 2005-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DEVICE_P_H
#define SOLID_DEVICE_P_H

#include <QMap>
#include <QObject>
#include <QPointer>
#include <QSharedData>

#include <ifaces/device.h>

#if defined(Q_OS_WIN32)
#undef interface
#endif

namespace Solid
{
class DevicePrivate : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DevicePrivate(const QString &udi);
    ~DevicePrivate() override;

    QString udi() const
    {
        return m_udi;
    }

    Ifaces::Device *backendObject() const
    {
        return m_backendObject.data();
    }
    void setBackendObject(Ifaces::Device *object);

    DeviceInterface *interface(const DeviceInterface::Type &type) const;
    void setInterface(const DeviceInterface::Type &type, DeviceInterface *interface);

public Q_SLOTS:
    void _k_destroyed(QObject *object);

private:
    QString m_udi;
    QPointer<Ifaces::Device> m_backendObject;
    QMap<DeviceInterface::Type, QPointer<DeviceInterface>> m_ifaces;
};
}

#endif
