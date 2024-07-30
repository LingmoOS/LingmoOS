/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakecamera.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeCamera::FakeCamera(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeCamera::~FakeCamera()
{
}

QStringList FakeCamera::supportedProtocols() const
{
    QStringList res;
    QString method = fakeDevice()->property("accessMethod").toString();

    res << method;

    return res;
}

QStringList FakeCamera::supportedDrivers(QString /*protocol*/) const
{
    QStringList res;

    if (fakeDevice()->property("gphotoSupport").toBool()) {
        res << "gphoto";
    }

    return res;
}

QVariant Solid::Backends::Fake::FakeCamera::driverHandle(const QString &driver) const
{
    Q_UNUSED(driver);
    return QVariant();
}

#include "moc_fakecamera.cpp"
