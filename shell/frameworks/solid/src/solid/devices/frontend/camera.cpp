/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "camera.h"
#include "camera_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/camera.h>

Solid::Camera::Camera(QObject *backendObject)
    : DeviceInterface(*new CameraPrivate(), backendObject)
{
}

Solid::Camera::~Camera()
{
}

QStringList Solid::Camera::supportedProtocols() const
{
    Q_D(const Camera);
    return_SOLID_CALL(Ifaces::Camera *, d->backendObject(), QStringList(), supportedProtocols());
}

QStringList Solid::Camera::supportedDrivers(QString protocol) const
{
    Q_D(const Camera);
    return_SOLID_CALL(Ifaces::Camera *, d->backendObject(), QStringList(), supportedDrivers(protocol));
}

QVariant Solid::Camera::driverHandle(const QString &driver) const
{
    Q_D(const Camera);
    return_SOLID_CALL(Ifaces::Camera *, d->backendObject(), QVariant(), driverHandle(driver));
}

#include "moc_camera.cpp"
