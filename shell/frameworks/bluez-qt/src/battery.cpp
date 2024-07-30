/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "battery.h"
#include "battery_p.h"
#include "macros.h"
#include "utils.h"

#include <QVariantMap>

namespace BluezQt
{
BatteryPrivate::BatteryPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
    , m_path(path)
{
    // Init properties
    m_percentage = properties.value(QStringLiteral("Percentage")).toInt();
}

void BatteryPrivate::propertiesChanged(const QString &interface, const QVariantMap &changed, const QStringList &invalidated)
{
    Q_UNUSED(invalidated)

    if (interface != Strings::orgBluezBattery1()) {
        return;
    }

    QVariantMap::const_iterator i;
    for (i = changed.constBegin(); i != changed.constEnd(); ++i) {
        const QVariant &value = i.value();
        const QString &property = i.key();

        if (property == QLatin1String("Percentage")) {
            PROPERTY_CHANGED2(m_percentage, value.toInt(), percentageChanged)
        }
    }
}

Battery::Battery(const QString &path, const QVariantMap &properties)
    : d(new BatteryPrivate(path, properties))
{
}

Battery::~Battery() = default;

BatteryPtr Battery::toSharedPtr() const
{
    return d->q.toStrongRef();
}

int Battery::percentage() const
{
    return d->m_percentage;
}

} // namespace BluezQt

#include "moc_battery.cpp"
#include "moc_battery_p.cpp"
