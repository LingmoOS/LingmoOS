/*
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FAKEHW_FAKEBATTERY_H
#define SOLID_BACKENDS_FAKEHW_FAKEBATTERY_H

#include "fakedeviceinterface.h"
#include <solid/devices/ifaces/battery.h>

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeBattery : public FakeDeviceInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    explicit FakeBattery(FakeDevice *device);
    ~FakeBattery() override;

public Q_SLOTS:
    bool isPresent() const override;
    Solid::Battery::BatteryType type() const override;

    int chargePercent() const override;
    int capacity() const override;

    bool isRechargeable() const override;
    bool isPowerSupply() const override;

    Solid::Battery::ChargeState chargeState() const override;
    qlonglong timeToEmpty() const override;
    qlonglong timeToFull() const override;

    void setChargeState(Solid::Battery::ChargeState newState);
    void setChargeLevel(int newLevel);

    Solid::Battery::Technology technology() const override;

    double energy() const override;
    double energyFull() const override;
    double energyFullDesign() const override;
    double energyRate() const override;

    double voltage() const override;
    double temperature() const override;

    QString serial() const override;

    qlonglong remainingTime() const override;

Q_SIGNALS:
    void presentStateChanged(bool newState, const QString &udi) override;
    void chargePercentChanged(int value, const QString &udi) override;
    void capacityChanged(int value, const QString &udi) override;
    void powerSupplyStateChanged(bool newState, const QString &udi) override;
    void chargeStateChanged(int newState, const QString &udi) override;
    void timeToEmptyChanged(qlonglong time, const QString &udi) override;
    void timeToFullChanged(qlonglong time, const QString &udi) override;
    void energyChanged(double energy, const QString &udi) override;
    void energyFullChanged(double energyFull, const QString &udi) override;
    void energyFullDesignChanged(double energyFullDesign, const QString &udi) override;
    void energyRateChanged(double energyRate, const QString &udi) override;
    void voltageChanged(double voltage, const QString &udi) override;
    void temperatureChanged(double temperature, const QString &udi) override;
    void remainingTimeChanged(qlonglong time, const QString &udi) override;
};
}
}
}

#endif // SOLID_BACKENDS_FAKEHW_FAKEBATTERY_H
