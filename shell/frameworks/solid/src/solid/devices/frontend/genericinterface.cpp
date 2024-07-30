/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "genericinterface.h"
#include "genericinterface_p.h"
#include "soliddefs_p.h"

#include <solid/devices/ifaces/genericinterface.h>

Solid::GenericInterface::GenericInterface(QObject *backendObject)
    : DeviceInterface(*new GenericInterfacePrivate(), backendObject)
{
    if (backendObject) {
        connect(backendObject, SIGNAL(propertyChanged(QMap<QString, int>)), this, SIGNAL(propertyChanged(QMap<QString, int>)));
        connect(backendObject, SIGNAL(conditionRaised(QString, QString)), this, SIGNAL(conditionRaised(QString, QString)));
    }
}

Solid::GenericInterface::~GenericInterface()
{
}

QVariant Solid::GenericInterface::property(const QString &key) const
{
    Q_D(const GenericInterface);
    return_SOLID_CALL(Ifaces::GenericInterface *, d->backendObject(), QVariant(), property(key));
}

QMap<QString, QVariant> Solid::GenericInterface::allProperties() const
{
    Q_D(const GenericInterface);
    return_SOLID_CALL(Ifaces::GenericInterface *, d->backendObject(), QVariantMap(), allProperties());
}

bool Solid::GenericInterface::propertyExists(const QString &key) const
{
    Q_D(const GenericInterface);
    return_SOLID_CALL(Ifaces::GenericInterface *, d->backendObject(), false, propertyExists(key));
}

#include "moc_genericinterface.cpp"
