/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#pragma once

#include <QDBusObjectPath>
#include <QObject>

#include "powerdevilcore_export.h"
#include "upower_interface.h"
#include "upowerdevice.h"

class POWERDEVILCORE_EXPORT BatteryController : public QObject
{
    Q_OBJECT
public:
    explicit BatteryController();

    /**
     * This enum type defines the different states of the AC adapter.
     *
     * - UnknownAcAdapterState: The AC adapter has an unknown state
     * - Plugged: The AC adapter is plugged
     * - Unplugged: The AC adapter is unplugged
     */
    enum AcAdapterState {
        UnknownAcAdapterState,
        Plugged,
        Unplugged,
    };
    Q_ENUM(AcAdapterState)

    /**
     * Retrieves the current state of the system AC adapter.
     *
     * @return the current AC adapter state
     * @see BatteryController::AcAdapterState
     */
    AcAdapterState acAdapterState() const;

    /**
     * Retrieves the current estimated remaining time of the system batteries
     *
     * @return the current global estimated remaining time in milliseconds
     */
    qulonglong batteryRemainingTime() const;

    /**
     * Retrieves the current estimated remaining time of the system batteries,
     * with exponential moving average filter applied to the history records.
     *
     * @return the current global estimated remaining time in milliseconds
     */
    qulonglong smoothedBatteryRemainingTime() const;

Q_SIGNALS:

    /**
     * This signal is emitted when the AC adapter is plugged or unplugged.
     *
     * @param newState the new state of the AC adapter, it's one of the
     * type @see PowerDevil::BackendInterface::AcAdapterState
     */
    void acAdapterStateChanged(BatteryController::AcAdapterState newState);

    /**
     * This signal is emitted when the estimated battery remaining time changes.
     *
     * @param time the new remaining time
     */
    void batteryRemainingTimeChanged(qulonglong time);

    /**
     * This signal is emitted when the estimated battery remaining time changes.
     *
     * @param time the new remaining time
     */
    void smoothedBatteryRemainingTimeChanged(qulonglong time);

private:
    void addDevice(const QString &);
    void setBatteryRate(const double rate, qulonglong timestamp);

private Q_SLOTS:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);
    void updateDeviceProps();
    void slotDeviceAdded(const QDBusObjectPath &path);
    void slotDeviceRemoved(const QDBusObjectPath &path);

private:
    // upower devices
    std::map<QString, std::unique_ptr<UPowerDevice>> m_devices;
    std::unique_ptr<UPowerDevice> m_displayDevice = nullptr;

    OrgFreedesktopUPowerInterface *m_upowerInterface;

    bool m_onBattery = false;

    AcAdapterState m_acAdapterState = UnknownAcAdapterState;

    qulonglong m_batteryRemainingTime = 0;
    qulonglong m_smoothedBatteryRemainingTime = 0;
    qulonglong m_lastRateTimestamp = 0;
    double m_batteryEnergyFull = 0;
    double m_batteryEnergy = 0;
    double m_smoothedBatteryDischargeRate = 0;
};
