/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DEVICEINTERFACE_P_H
#define SOLID_DEVICEINTERFACE_P_H

#include <QPointer>

namespace Solid
{
class DevicePrivate;

class DeviceInterfacePrivate
{
public:
    DeviceInterfacePrivate();
    virtual ~DeviceInterfacePrivate();

    QObject *backendObject() const;
    void setBackendObject(QObject *object);
    DevicePrivate *devicePrivate() const;
    void setDevicePrivate(DevicePrivate *devicePrivate);

private:
    QPointer<QObject> m_backendObject;
    DevicePrivate *m_devicePrivate;
};
}

#endif
