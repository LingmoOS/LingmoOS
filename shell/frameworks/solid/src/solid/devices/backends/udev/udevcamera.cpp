/*
    SPDX-FileCopyrightText: 2010 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udevcamera.h"

using namespace Solid::Backends::UDev;

Camera::Camera(UDevDevice *device)
    : DeviceInterface(device)
{
}

Camera::~Camera()
{
}

QStringList Camera::supportedProtocols() const
{
    QStringList protocols;

    QString method = m_device->property("GPHOTO2_DRIVER").toString();

    if (!method.isEmpty()) {
        protocols << method.toLower();
    }

    return protocols;
}

QStringList Camera::supportedDrivers(QString /*protocol*/) const
{
    QStringList res;

    if (!supportedProtocols().isEmpty()) {
        res << "gphoto";
    }

    return res;
}

QVariant Camera::driverHandle(const QString &driver) const
{
    if (driver == "gphoto" && m_device->property("SUBSYSTEM").toString() == "usb") {
        QVariantList list;

        list << "usb" << m_device->property("ID_VENDOR_ID") << m_device->property("ID_MODEL_ID");

        return list;
    }

    return QVariant();
}

#include "moc_udevcamera.cpp"
