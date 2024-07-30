/*
    SPDX-FileCopyrightText: 2009 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_IOKIT_BATTERY_H
#define SOLID_BACKENDS_IOKIT_BATTERY_H

#include "iokitdeviceinterface.h"
#include <solid/devices/ifaces/battery.h>

namespace Solid
{
namespace Backends
{
namespace IOKit
{
class IOKitDevice;

class Battery : public DeviceInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    Battery(IOKitDevice *device);
    virtual ~Battery();

    bool isPresent() const override;
    Solid::Battery::BatteryType type() const override;

    int chargePercent() const override;
    int capacity() const override;

    bool isRechargeable() const override;
    bool isPowerSupply() const override;

    Solid::Battery::ChargeState chargeState() const override;

    qlonglong timeToEmpty() const override;
    qlonglong timeToFull() const override;
    double voltage() const override;
    double temperature() const override;
    QString serial() const override;

    // ### the ones below are TODO
    // clang-format off
    Solid::Battery::Technology technology() const override { return Solid::Battery::UnknownTechnology; }
    double energy() const override { return 0.0; }
    double energyFull() const override { return 0.0; }
    double energyFullDesign() const override { return 0.0; }
    double energyRate() const override { return 0.0; }

    qlonglong remainingTime() const override { return -1; }
    // clang-format on

Q_SIGNALS:
    void energyChanged(double energy, const QString &udi) override;
    void energyFullChanged(double energyFull, const QString &udi) override;
    void energyFullDesignChanged(double energyFullDesign, const QString &udi) override;
    void energyRateChanged(double energyRate, const QString &udi) override;
    void chargePercentChanged(int value, const QString &udi) override;
    void capacityChanged(int value, const QString &udi) override;
    void chargeStateChanged(int newState, const QString &udi) override;
    void presentStateChanged(bool newState, const QString &udi) override;
    void powerSupplyStateChanged(bool newState, const QString &udi) override;
    void timeToEmptyChanged(qlonglong time, const QString &udi) override;
    void timeToFullChanged(qlonglong time, const QString &udi) override;
    void temperatureChanged(double temperature, const QString &udi) override;
    void voltageChanged(double voltage, const QString &udi) override;
    void remainingTimeChanged(qlonglong time, const QString &udi) override;
};
}
}
}

#endif // SOLID_BACKENDS_IOKIT_BATTERY_H
