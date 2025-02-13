/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@broulik.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "declarativebattery.h"

DeclarativeBattery::DeclarativeBattery(const BluezQt::BatteryPtr &battery, QObject *parent)
    : QObject(parent)
    , m_battery(battery)
{
    connect(m_battery.data(), &BluezQt::Battery::percentageChanged, this, &DeclarativeBattery::percentageChanged);
}

int DeclarativeBattery::percentage() const
{
    return m_battery->percentage();
}
