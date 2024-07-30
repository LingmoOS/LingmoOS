/*
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BATTERYINTERFACE_H
#define BATTERYINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusObjectPath;

class BatteryObject : public QObject
{
public:
    explicit BatteryObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class BatteryInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Battery1")
    Q_PROPERTY(uchar Percentage READ percentage)

public:
    explicit BatteryInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    uchar percentage() const;
};

#endif // BATTERYINTERFACE_H
