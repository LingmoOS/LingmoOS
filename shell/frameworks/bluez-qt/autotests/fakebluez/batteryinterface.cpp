/*
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "batteryinterface.h"

#include <QDBusConnection>

// BatteryObject
BatteryObject::BatteryObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// BatteryInterface
BatteryInterface::BatteryInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.Battery1"));
}

uchar BatteryInterface::percentage() const
{
    return Object::property(QStringLiteral("Percentage")).value<uchar>();
}

#include "moc_batteryinterface.cpp"
