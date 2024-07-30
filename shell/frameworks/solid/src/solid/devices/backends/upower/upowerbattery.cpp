/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>
    SPDX-FileCopyrightText: 2010, 2012, 2015 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "upowerbattery.h"

#include "upower.h"

using namespace Solid::Backends::UPower;

Battery::Battery(UPowerDevice *device)
    : DeviceInterface(device)
{
    connect(device, &UPowerDevice::propertyChanged, this, &Battery::slotChanged);

    updateCache();
}

Battery::~Battery()
{
}

bool Battery::isPresent() const
{
    return m_device.data()->prop("IsPresent").toBool();
}

Solid::Battery::BatteryType Battery::type() const
{
    Solid::Battery::BatteryType result = Solid::Battery::UnknownBattery;
    const auto t = static_cast<UpDeviceKind>(m_device.data()->prop("Type").toUInt());
    switch (t) {
    case UP_DEVICE_KIND_LINE_POWER: // TODO
        break;
    case UP_DEVICE_KIND_BATTERY:
        result = Solid::Battery::PrimaryBattery;
        break;
    case UP_DEVICE_KIND_UPS:
        result = Solid::Battery::UpsBattery;
        break;
    case UP_DEVICE_KIND_MONITOR:
        result = Solid::Battery::MonitorBattery;
        break;
    case UP_DEVICE_KIND_MOUSE:
        result = Solid::Battery::MouseBattery;
        break;
    case UP_DEVICE_KIND_KEYBOARD:
        result = Solid::Battery::KeyboardBattery;
        break;
    case UP_DEVICE_KIND_PDA:
        result = Solid::Battery::PdaBattery;
        break;
    case UP_DEVICE_KIND_PHONE:
        result = Solid::Battery::PhoneBattery;
        break;
    case UP_DEVICE_KIND_TABLET:
        result = Solid::Battery::TabletBattery;
        break;
    case UP_DEVICE_KIND_GAMING_INPUT:
        result = Solid::Battery::GamingInputBattery;
        break;
    case UP_DEVICE_KIND_HEADPHONES:
        result = Solid::Battery::HeadphoneBattery;
        break;
    case UP_DEVICE_KIND_HEADSET:
        result = Solid::Battery::HeadsetBattery;
        break;
    case UP_DEVICE_KIND_TOUCHPAD:
        result = Solid::Battery::TouchpadBattery;
        break;
    case UP_DEVICE_KIND_BLUETOOTH_GENERIC:
        result = Solid::Battery::BluetoothBattery;
        break;
    case UP_DEVICE_KIND_UNKNOWN:
        break;
    }

    if (result == Solid::Battery::UnknownBattery) {
        // Check if the battery came from Bluez, which is more useful than unknown battery type
        // UP_DEVICE_KIND_BLUETOOTH_GENERIC is only in UPower 0.99.12
        if (m_device.data()->prop("NativePath").toString().startsWith(QLatin1String("/org/bluez/"))) {
            result = Solid::Battery::BluetoothBattery;
        }
    }

    return result;
}

int Battery::chargePercent() const
{
    return qRound(m_device.data()->prop("Percentage").toDouble());
}

int Battery::capacity() const
{
    return qRound(m_device.data()->prop("Capacity").toDouble());
}

bool Battery::isRechargeable() const
{
    return m_device.data()->prop("IsRechargeable").toBool();
}

bool Battery::isPowerSupply() const
{
    return m_device.data()->prop("PowerSupply").toBool();
}

Solid::Battery::ChargeState Battery::chargeState() const
{
    Solid::Battery::ChargeState result = Solid::Battery::NoCharge;
    const UpDeviceState state = static_cast<UpDeviceState>(m_device.data()->prop("State").toUInt());
    switch (state) {
    case UP_DEVICE_STATE_UNKNOWN:
        result = Solid::Battery::NoCharge; // stable or unknown
        break;
    case UP_DEVICE_STATE_CHARGING:
        result = Solid::Battery::Charging;
        break;
    case UP_DEVICE_STATE_DISCHARGING:
        result = Solid::Battery::Discharging;
        break;
    case UP_DEVICE_STATE_EMPTY: // TODO "Empty"
        break;
    case UP_DEVICE_STATE_FULLY_CHARGED:
        result = Solid::Battery::FullyCharged;
        break;
    case UP_DEVICE_STATE_PENDING_CHARGE: // TODO "Pending charge"
        break;
    case UP_DEVICE_STATE_PENDING_DISCHARGE: // TODO "Pending discharge"
        break;
    case UP_DEVICE_STATE_LAST:
        break;
    }
    return result;
}

qlonglong Battery::timeToEmpty() const
{
    return m_device.data()->prop("TimeToEmpty").toLongLong();
}

qlonglong Battery::timeToFull() const
{
    return m_device.data()->prop("TimeToFull").toLongLong();
}

Solid::Battery::Technology Battery::technology() const
{
    const UpDeviceTechnology tech = static_cast<UpDeviceTechnology>(m_device.data()->prop("Technology").toUInt());
    switch (tech) {
    case UP_DEVICE_TECHNOLOGY_UNKNOWN:
        return Solid::Battery::UnknownTechnology;
    case UP_DEVICE_TECHNOLOGY_LITHIUM_ION:
        return Solid::Battery::LithiumIon;
    case UP_DEVICE_TECHNOLOGY_LITHIUM_POLYMER:
        return Solid::Battery::LithiumPolymer;
    case UP_DEVICE_TECHNOLOGY_LITHIUM_IRON_PHOSPHATE:
        return Solid::Battery::LithiumIronPhosphate;
    case UP_DEVICE_TECHNOLOGY_LEAD_ACID:
        return Solid::Battery::LeadAcid;
    case UP_DEVICE_TECHNOLOGY_NICKEL_CADMIUM:
        return Solid::Battery::NickelCadmium;
    case UP_DEVICE_TECHNOLOGY_NICKEL_METAL_HYDRIDE:
        return Solid::Battery::NickelMetalHydride;
    case UP_DEVICE_TECHNOLOGY_LAST:
        return Solid::Battery::UnknownTechnology;
    }
    return Solid::Battery::UnknownTechnology;
}

double Battery::energy() const
{
    return m_device.data()->prop("Energy").toDouble();
}

double Battery::energyFull() const
{
    return m_device.data()->prop("EnergyFull").toDouble();
}

double Battery::energyFullDesign() const
{
    return m_device.data()->prop("EnergyFullDesign").toDouble();
}

double Battery::energyRate() const
{
    return m_device.data()->prop("EnergyRate").toDouble();
}

double Battery::voltage() const
{
    return m_device.data()->prop("Voltage").toDouble();
}

double Battery::temperature() const
{
    return m_device.data()->prop("Temperature").toDouble();
}

QString Battery::serial() const
{
    return m_device.data()->prop("Serial").toString();
}

qlonglong Battery::remainingTime() const
{
    if (chargeState() == Solid::Battery::Charging) {
        return timeToFull();
    } else if (chargeState() == Solid::Battery::Discharging) {
        return timeToEmpty();
    }

    return -1;
}

void Battery::slotChanged()
{
    if (m_device) {
        const bool old_isPresent = m_isPresent;
        const int old_chargePercent = m_chargePercent;
        const int old_capacity = m_capacity;
        const bool old_isPowerSupply = m_isPowerSupply;
        const Solid::Battery::ChargeState old_chargeState = m_chargeState;
        const qlonglong old_timeToEmpty = m_timeToEmpty;
        const qlonglong old_timeToFull = m_timeToFull;
        const double old_energy = m_energy;
        const double old_energyFull = m_energyFull;
        const double old_energyFullDesign = m_energyFullDesign;
        const double old_energyRate = m_energyRate;
        const double old_voltage = m_voltage;
        const double old_temperature = m_temperature;
        updateCache();

        if (old_isPresent != m_isPresent) {
            Q_EMIT presentStateChanged(m_isPresent, m_device.data()->udi());
        }

        if (old_chargePercent != m_chargePercent) {
            Q_EMIT chargePercentChanged(m_chargePercent, m_device.data()->udi());
        }

        if (old_capacity != m_capacity) {
            Q_EMIT capacityChanged(m_capacity, m_device.data()->udi());
        }

        if (old_isPowerSupply != m_isPowerSupply) {
            Q_EMIT powerSupplyStateChanged(m_isPowerSupply, m_device.data()->udi());
        }

        if (old_chargeState != m_chargeState) {
            Q_EMIT chargeStateChanged(m_chargeState, m_device.data()->udi());
        }

        if (old_timeToEmpty != m_timeToEmpty) {
            Q_EMIT timeToEmptyChanged(m_timeToEmpty, m_device.data()->udi());
        }

        if (old_timeToFull != m_timeToFull) {
            Q_EMIT timeToFullChanged(m_timeToFull, m_device.data()->udi());
        }

        if (old_energy != m_energy) {
            Q_EMIT energyChanged(m_energy, m_device.data()->udi());
        }

        if (old_energyFull != m_energyFull) {
            Q_EMIT energyFullChanged(m_energyFull, m_device.data()->udi());
        }

        if (old_energyFullDesign != m_energyFullDesign) {
            Q_EMIT energyFullChanged(m_energyFullDesign, m_device.data()->udi());
        }

        if (old_energyRate != m_energyRate) {
            Q_EMIT energyRateChanged(m_energyRate, m_device.data()->udi());
        }

        if (old_voltage != m_voltage) {
            Q_EMIT voltageChanged(m_voltage, m_device.data()->udi());
        }

        if (old_temperature != m_temperature) {
            Q_EMIT temperatureChanged(m_temperature, m_device.data()->udi());
        }

        if (old_timeToFull != m_timeToFull || old_timeToEmpty != m_timeToEmpty) {
            Q_EMIT remainingTimeChanged(remainingTime(), m_device.data()->udi());
        }
    }
}

void Battery::updateCache()
{
    m_isPresent = isPresent();
    m_chargePercent = chargePercent();
    m_capacity = capacity();
    m_isPowerSupply = isPowerSupply();
    m_chargeState = chargeState();
    m_timeToEmpty = timeToEmpty();
    m_timeToFull = timeToFull();
    m_energy = energy();
    m_energyFull = energyFull();
    m_energyFullDesign = energyFullDesign();
    m_energyRate = energyRate();
    m_voltage = voltage();
    m_temperature = temperature();
}

#include "moc_upowerbattery.cpp"
