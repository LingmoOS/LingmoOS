/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fakebattery.h"
#include <QVariant>

using namespace Solid::Backends::Fake;

FakeBattery::FakeBattery(FakeDevice *device)
    : FakeDeviceInterface(device)
{
}

FakeBattery::~FakeBattery()
{
}

bool FakeBattery::isPresent() const
{
    return fakeDevice()->property("isPresent").toBool();
}

Solid::Battery::BatteryType FakeBattery::type() const
{
    QString name = fakeDevice()->property("batteryType").toString();

    if (name == "pda") {
        return Solid::Battery::PdaBattery;
    } else if (name == "ups") {
        return Solid::Battery::UpsBattery;
    } else if (name == "primary") {
        return Solid::Battery::PrimaryBattery;
    } else if (name == "mouse") {
        return Solid::Battery::MouseBattery;
    } else if (name == "keyboard") {
        return Solid::Battery::KeyboardBattery;
    } else if (name == "keyboard_mouse") {
        return Solid::Battery::KeyboardMouseBattery;
    } else if (name == "camera") {
        return Solid::Battery::CameraBattery;
    } else if (name == "gaminginput") {
        return Solid::Battery::GamingInputBattery;
    } else if (name == "bluetooth") {
        return Solid::Battery::BluetoothBattery;
    } else if (name == "tablet") {
        return Solid::Battery::TabletBattery;
    } else {
        return Solid::Battery::UnknownBattery;
    }
}

int FakeBattery::chargePercent() const
{
    int last_full = fakeDevice()->property("lastFullLevel").toInt();
    int current = fakeDevice()->property("currentLevel").toInt();

    int percent = 0;
    if (last_full > 0) {
        percent = (100 * current) / last_full;
    }

    return percent;
}

int FakeBattery::capacity() const
{
    return fakeDevice()->property("capacity").toInt();
}

bool FakeBattery::isRechargeable() const
{
    return fakeDevice()->property("isRechargeable").toBool();
}

bool FakeBattery::isPowerSupply() const
{
    return fakeDevice()->property("isPowerSupply").toBool();
}

Solid::Battery::ChargeState FakeBattery::chargeState() const
{
    QString state = fakeDevice()->property("chargeState").toString();

    if (state == "charging") {
        return Solid::Battery::Charging;
    } else if (state == "discharging") {
        return Solid::Battery::Discharging;
    } else if (state == "fullyCharged") {
        return Solid::Battery::FullyCharged;
    } else {
        return Solid::Battery::NoCharge;
    }
}

qlonglong FakeBattery::timeToEmpty() const
{
    return fakeDevice()->property("timeToEmpty").toLongLong();
}

qlonglong FakeBattery::timeToFull() const
{
    return fakeDevice()->property("timeToFull").toLongLong();
}

void FakeBattery::setChargeState(Solid::Battery::ChargeState newState)
{
    QString name;

    switch (newState) {
    case Solid::Battery::Charging:
        name = "charging";
        break;
    case Solid::Battery::Discharging:
        name = "discharging";
        break;
    case Solid::Battery::NoCharge:
        name = "noCharge";
        break;
    case Solid::Battery::FullyCharged:
        name = "fullyCharged";
        break;
    }

    fakeDevice()->setProperty("chargeState", name);
    Q_EMIT chargeStateChanged(newState, fakeDevice()->udi());
}

void FakeBattery::setChargeLevel(int newLevel)
{
    fakeDevice()->setProperty("currentLevel", newLevel);
    Q_EMIT chargePercentChanged(chargePercent(), fakeDevice()->udi());
}

Solid::Battery::Technology FakeBattery::technology() const
{
    return (Solid::Battery::Technology)fakeDevice()->property("technology").toInt();
}

double FakeBattery::energy() const
{
    return fakeDevice()->property("energy").toDouble();
}

double FakeBattery::energyFull() const
{
    return fakeDevice()->property("energyFull").toDouble();
}

double FakeBattery::energyFullDesign() const
{
    return fakeDevice()->property("energyFullDesign").toDouble();
}

double FakeBattery::energyRate() const
{
    return fakeDevice()->property("energyRate").toDouble();
}

double FakeBattery::voltage() const
{
    return fakeDevice()->property("voltage").toDouble();
}

double FakeBattery::temperature() const
{
    return fakeDevice()->property("temperature").toDouble();
}

QString FakeBattery::serial() const
{
    return fakeDevice()->property("serial").toString();
}

qlonglong FakeBattery::remainingTime() const
{
    return fakeDevice()->property("remainingTime").toLongLong();
}

#include "moc_fakebattery.cpp"
