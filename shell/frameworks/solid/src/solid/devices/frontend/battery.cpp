/*
    SPDX-FileCopyrightText: 2006-2007 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "battery.h"
#include "battery_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/battery.h>

Solid::Battery::Battery(QObject *backendObject)
    : DeviceInterface(*new BatteryPrivate(), backendObject)
{
    connect(backendObject, SIGNAL(presentStateChanged(bool, QString)), this, SIGNAL(presentStateChanged(bool, QString)));

    connect(backendObject, SIGNAL(chargePercentChanged(int, QString)), this, SIGNAL(chargePercentChanged(int, QString)));

    connect(backendObject, SIGNAL(capacityChanged(int, QString)), this, SIGNAL(capacityChanged(int, QString)));

    connect(backendObject, SIGNAL(powerSupplyStateChanged(bool, QString)), this, SIGNAL(powerSupplyStateChanged(bool, QString)));

    connect(backendObject, SIGNAL(chargeStateChanged(int, QString)), this, SIGNAL(chargeStateChanged(int, QString)));

    connect(backendObject, SIGNAL(timeToEmptyChanged(qlonglong, QString)), this, SIGNAL(timeToEmptyChanged(qlonglong, QString)));

    connect(backendObject, SIGNAL(timeToFullChanged(qlonglong, QString)), this, SIGNAL(timeToFullChanged(qlonglong, QString)));

    connect(backendObject, SIGNAL(energyChanged(double, QString)), this, SIGNAL(energyChanged(double, QString)));

    connect(backendObject, SIGNAL(energyFullChanged(double, QString)), this, SIGNAL(energyFullChanged(double, QString)));

    connect(backendObject, SIGNAL(energyFullDesignChanged(double, QString)), this, SIGNAL(energyFullDesignChanged(double, QString)));

    connect(backendObject, SIGNAL(energyRateChanged(double, QString)), this, SIGNAL(energyRateChanged(double, QString)));

    connect(backendObject, SIGNAL(voltageChanged(double, QString)), this, SIGNAL(voltageChanged(double, QString)));

    connect(backendObject, SIGNAL(temperatureChanged(double, QString)), this, SIGNAL(temperatureChanged(double, QString)));

    connect(backendObject, SIGNAL(remainingTimeChanged(qlonglong, QString)), this, SIGNAL(remainingTimeChanged(qlonglong, QString)));
}

Solid::Battery::~Battery()
{
}

bool Solid::Battery::isPresent() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), false, isPresent());
}

Solid::Battery::BatteryType Solid::Battery::type() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), UnknownBattery, type());
}

int Solid::Battery::chargePercent() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0, chargePercent());
}

int Solid::Battery::capacity() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 100, capacity());
}

bool Solid::Battery::isRechargeable() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), false, isRechargeable());
}

bool Solid::Battery::isPowerSupply() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), true, isPowerSupply());
}

Solid::Battery::ChargeState Solid::Battery::chargeState() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), NoCharge, chargeState());
}

qlonglong Solid::Battery::timeToEmpty() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0, timeToEmpty());
}

qlonglong Solid::Battery::timeToFull() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0, timeToFull());
}

Solid::Battery::Technology Solid::Battery::technology() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), UnknownTechnology, technology());
}

double Solid::Battery::energy() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0.0, energy());
}

double Solid::Battery::energyFull() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0.0, energyFull());
}

double Solid::Battery::energyFullDesign() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0.0, energyFullDesign());
}

double Solid::Battery::energyRate() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0.0, energyRate());
}

double Solid::Battery::voltage() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0.0, voltage());
}

double Solid::Battery::temperature() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), 0.0, temperature());
}

QString Solid::Battery::serial() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), QString(), serial());
}

qlonglong Solid::Battery::remainingTime() const
{
    Q_D(const Battery);
    return_SOLID_CALL(Ifaces::Battery *, d->backendObject(), -1, remainingTime());
}

#include "moc_battery.cpp"
