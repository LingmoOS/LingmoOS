/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#include "batterycontroller.h"

#include <QDBusConnection>

using namespace Qt::StringLiterals;

inline constexpr QLatin1StringView UPOWER_SERVICE("org.freedesktop.UPower");
inline constexpr QLatin1StringView UPOWER_PATH("/org/freedesktop/UPower");
inline constexpr QLatin1StringView UPOWER_IFACE("org.freedesktop.UPower");
inline constexpr QLatin1StringView UPOWER_IFACE_DEVICE("org.freedesktop.UPower.Device");

BatteryController::BatteryController()
    : QObject()
{
    QDBusConnection::systemBus().connect(UPOWER_SERVICE,
                                         UPOWER_PATH,
                                         u"org.freedesktop.DBus.Properties"_s,
                                         u"PropertiesChanged"_s,
                                         this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

    QDBusConnection::systemBus().connect(UPOWER_SERVICE, UPOWER_PATH, UPOWER_IFACE, u"DeviceAdded"_s, this, SLOT(slotDeviceAdded(QDBusObjectPath)));
    QDBusConnection::systemBus().connect(UPOWER_SERVICE, UPOWER_PATH, UPOWER_IFACE, u"DeviceRemoved"_s, this, SLOT(slotDeviceRemoved(QDBusObjectPath)));

    m_upowerInterface = new OrgFreedesktopUPowerInterface(UPOWER_SERVICE, u"/org/freedesktop/UPower"_s, QDBusConnection::systemBus(), this);

    m_onBattery = m_upowerInterface->onBattery();

    QDBusReply<QDBusObjectPath> reply = m_upowerInterface->call(u"GetDisplayDevice"_s);
    if (reply.isValid()) {
        const QString path = reply.value().path();
        if (!path.isEmpty() && path != QStringLiteral("/")) {
            m_displayDevice = std::make_unique<UPowerDevice>(path);
            connect(m_displayDevice.get(), &UPowerDevice::propertiesChanged, this, &BatteryController::updateDeviceProps);
        }
    }

    if (!m_displayDevice) {
        const QList<QDBusObjectPath> deviceList = m_upowerInterface->EnumerateDevices();
        for (const QDBusObjectPath &device : deviceList) {
            if (m_devices.count(device.path())) {
                continue;
            }
            addDevice(device.path());
        }
    }

    updateDeviceProps();

    if (m_onBattery) {
        m_acAdapterState = Unplugged;
    } else {
        m_acAdapterState = Plugged;
    }
    Q_EMIT acAdapterStateChanged(m_acAdapterState);
}

void BatteryController::addDevice(const QString &device)
{
    if (m_displayDevice) {
        return;
    }

    auto upowerDevice = std::make_unique<UPowerDevice>(device);
    connect(upowerDevice.get(), &UPowerDevice::propertiesChanged, this, &BatteryController::updateDeviceProps);

    m_devices[device] = std::move(upowerDevice);
}

void BatteryController::slotDeviceAdded(const QDBusObjectPath &path)
{
    addDevice(path.path());
    updateDeviceProps();
}

void BatteryController::slotDeviceRemoved(const QDBusObjectPath &path)
{
    m_devices.erase(path.path());
    updateDeviceProps();
}

void BatteryController::updateDeviceProps()
{
    double energyTotal = 0.0;
    double energyRateTotal = 0.0; // +: charging, -: discharging (contrary to EnergyRate)
    double energyFullTotal = 0.0;
    qulonglong timestamp = 0;

    if (m_displayDevice) {
        if (!m_displayDevice->isPresent()) {
            // No Battery/Ups, nothing to report
            return;
        }
        const auto state = m_displayDevice->state();
        energyTotal = m_displayDevice->energy();
        energyFullTotal = m_displayDevice->energyFull();
        timestamp = m_displayDevice->updateTime();
        // Workaround for https://gitlab.freedesktop.org/upower/upower/-/issues/252
        if (state == UPowerDevice::State::Charging) {
            energyRateTotal = std::abs(m_displayDevice->energyRate());
        } else if (state == UPowerDevice::State::Discharging) {
            energyRateTotal = -std::abs(m_displayDevice->energyRate());
        }
    } else {
        for (const auto &[key, upowerDevice] : m_devices) {
            if (!upowerDevice->isPowerSupply() || !upowerDevice->isPresent()) {
                continue;
            }
            const auto type = upowerDevice->type();
            if (type == UPowerDevice::Type::Battery || type == UPowerDevice::Type::Ups) {
                const auto state = upowerDevice->state();
                energyFullTotal += upowerDevice->energyFull();
                energyTotal += upowerDevice->energy();

                if (state == UPowerDevice::State::FullyCharged) {
                    continue;
                }

                timestamp = std::max(timestamp, upowerDevice->updateTime());
                // Workaround for https://gitlab.freedesktop.org/upower/upower/-/issues/252
                if (state == UPowerDevice::State::Charging) {
                    energyRateTotal += std::abs(upowerDevice->energyRate());
                } else if (state == UPowerDevice::State::Discharging) {
                    energyRateTotal -= std::abs(upowerDevice->energyRate());
                }
            }
        }
    }

    m_batteryEnergy = energyTotal;
    m_batteryEnergyFull = energyFullTotal;
    setBatteryRate(energyRateTotal, timestamp);
}

void BatteryController::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    if (ifaceName != UPOWER_IFACE) {
        return;
    }

    bool onBattery = m_onBattery;
    if (changedProps.contains(QStringLiteral("OnBattery"))) {
        onBattery = changedProps[QStringLiteral("OnBattery")].toBool();
    } else if (invalidatedProps.contains(QStringLiteral("OnBattery"))) {
        onBattery = m_upowerInterface->onBattery();
    }
    if (onBattery != m_onBattery) {
        m_acAdapterState = onBattery ? Unplugged : Plugged;
        Q_EMIT acAdapterStateChanged(m_acAdapterState);
        m_onBattery = onBattery;
    }
}

/**
 * Filter data along one dimension using exponential moving average.
 */
double emafilter(const double last, const double update, double weight)
{
    double current = last * (1 - weight) + update * weight;

    return current;
}

void BatteryController::setBatteryRate(const double rate, qulonglong timestamp)
{
    // remaining time in milliseconds
    qulonglong time = 0;

    if (rate > 0) {
        // Energy and rate are in Watt*hours resp. Watt
        time = 3600 * 1000 * (m_batteryEnergyFull - m_batteryEnergy) / rate;
    } else if (rate < 0) {
        time = 3600 * 1000 * (0.0 - m_batteryEnergy) / rate;
    }

    if (m_batteryRemainingTime != time) {
        m_batteryRemainingTime = time;
        Q_EMIT batteryRemainingTimeChanged(time);
    }

    // Charging or full
    if ((rate > 0) || (time == 0)) {
        if (m_smoothedBatteryRemainingTime != time) {
            m_smoothedBatteryRemainingTime = time;
            Q_EMIT smoothedBatteryRemainingTimeChanged(time);
        }
        return;
    }

    double oldRate = m_smoothedBatteryDischargeRate;
    if (oldRate == 0) {
        m_smoothedBatteryDischargeRate = rate;
    } else {
        // To have a time constant independent from the update frequency
        // the weight must be scaled
        double weight = 0.005 * std::min<qulonglong>(60, timestamp - m_lastRateTimestamp);
        m_lastRateTimestamp = timestamp;
        m_smoothedBatteryDischargeRate = emafilter(oldRate, rate, weight);
    }

    time = 3600 * 1000 * (0.0 - m_batteryEnergy) / m_smoothedBatteryDischargeRate;

    if (m_smoothedBatteryRemainingTime != time) {
        m_smoothedBatteryRemainingTime = time;
        Q_EMIT smoothedBatteryRemainingTimeChanged(m_smoothedBatteryRemainingTime);
    }
}

BatteryController::AcAdapterState BatteryController::acAdapterState() const
{
    return m_acAdapterState;
}

qulonglong BatteryController::batteryRemainingTime() const
{
    return m_batteryRemainingTime;
}

qulonglong BatteryController::smoothedBatteryRemainingTime() const
{
    return m_smoothedBatteryRemainingTime;
}

#include "moc_batterycontroller.cpp"