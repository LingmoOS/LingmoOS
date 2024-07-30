/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>
    SPDX-FileCopyrightText: 2010, 2012 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_UPOWER_BATTERY_H
#define SOLID_BACKENDS_UPOWER_BATTERY_H

#include "upowerdeviceinterface.h"
#include <solid/devices/ifaces/battery.h>

namespace Solid
{
namespace Backends
{
namespace UPower
{
class Battery : public DeviceInterface, virtual public Solid::Ifaces::Battery
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::Battery)

public:
    Battery(UPowerDevice *device);
    ~Battery() override;

    bool isPresent() const override;

    Solid::Battery::BatteryType type() const override;

    int chargePercent() const override;

    int capacity() const override;

    bool isRechargeable() const override;

    bool isPowerSupply() const override;

    Solid::Battery::ChargeState chargeState() const override;

    qlonglong timeToEmpty() const override;

    qlonglong timeToFull() const override;

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
    void chargePercentChanged(int value, const QString &udi = QString()) override;
    void capacityChanged(int value, const QString &udi) override;
    void powerSupplyStateChanged(bool newState, const QString &udi) override;
    void chargeStateChanged(int newState, const QString &udi = QString()) override;
    void timeToEmptyChanged(qlonglong time, const QString &udi) override;
    void timeToFullChanged(qlonglong time, const QString &udi) override;
    void energyChanged(double energy, const QString &udi) override;
    void energyFullChanged(double energyFull, const QString &udi) override;
    void energyFullDesignChanged(double energyFullDesign, const QString &udi) override;
    void energyRateChanged(double energyRate, const QString &udi) override;
    void voltageChanged(double voltage, const QString &udi) override;
    void temperatureChanged(double temperature, const QString &udi) override;
    void remainingTimeChanged(qlonglong time, const QString &udi) override;

private Q_SLOTS:
    void slotChanged();

private:
    void updateCache();

    bool m_isPresent;
    int m_chargePercent;
    int m_capacity;
    bool m_isPowerSupply;
    Solid::Battery::ChargeState m_chargeState;
    qlonglong m_timeToEmpty;
    qlonglong m_timeToFull;
    double m_energy;
    double m_energyFull;
    double m_energyFullDesign;
    double m_energyRate;
    double m_voltage;
    double m_temperature;
};
}
}
}

#endif // SOLID_BACKENDS_UPOWER_BATTERY_H
