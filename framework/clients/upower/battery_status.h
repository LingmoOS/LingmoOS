// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::upower {

class Device;

// The battery as lingmo-services reports it, and whether the system runs
// on it (UPower's OnBattery). Everything is read asynchronously and kept
// here, so the getters never block.
class BatteryStatus
{
public:
    // One instance for everyone on the calling thread's loop, while anyone
    // holds it.
    static std::shared_ptr<BatteryStatus> shared();

    // Without a session bus, as at the login screen, the level and whether
    // there is a battery come from UPower's display device; the service's
    // other figures stay unset.
    explicit BatteryStatus(base::dbus::Connection* sessionBus = base::dbus::Connection::session(),
                           base::dbus::Connection* systemBus = base::dbus::Connection::system());
    ~BatteryStatus();

    BatteryStatus(const BatteryStatus&) = delete;
    BatteryStatus& operator=(const BatteryStatus&) = delete;

    // Whether the service has answered once; until then available() is false either way.
    bool isReady() const { return m_ready; }
    // lingmo-services publishes the battery object only when there is a battery.
    bool isAvailable() const { return m_available; }
    bool onBattery() const { return m_onBattery; }

    // The service's enum: NoCharge, Charging, Discharging, FullyCharged.
    int chargeState() const { return m_chargeState; }
    int chargePercent() const { return m_chargePercent; }
    int capacity() const { return m_capacity; }
    const std::string& statusString() const { return m_statusString; }
    int lastChargedPercent() const { return m_lastChargedPercent; }
    int64_t lastChargedSecs() const { return m_lastChargedSecs; }
    // The UPower device object path.
    const std::string& udi() const { return m_udi; }

    // Asks the service to read the battery again.
    void refresh();

    base::Signal<> readyChanged;
    base::Signal<> availableChanged;
    base::Signal<> onBatteryChanged;
    base::Signal<> chargeStateChanged;
    base::Signal<> chargePercentChanged;
    base::Signal<> capacityChanged;
    base::Signal<> statusStringChanged;
    base::Signal<> lastChargedChanged;
    base::Signal<> udiChanged;

private:
    void fetchState();
    void applyState(const base::dbus::VariantMap& properties);
    void applyDisplayDevice();
    void fetchOnBattery();
    void setOnBattery(bool onBattery);

    base::dbus::Proxy m_service;
    base::dbus::Proxy m_upower;
    std::unique_ptr<Device> m_displayDevice;
    std::vector<base::Subscription> m_displayDeviceSubscriptions;
    std::vector<base::dbus::Match> m_matches;
    base::dbus::PendingCall m_stateCall;
    base::dbus::PendingCall m_onBatteryCall;
    bool m_ready = false;
    bool m_available = false;
    bool m_onBattery = false;
    int m_chargeState = 0;
    int m_chargePercent = 0;
    int m_capacity = 0;
    std::string m_statusString;
    int m_lastChargedPercent = 0;
    int64_t m_lastChargedSecs = 0;
    std::string m_udi;
};

} // namespace Lingmo::upower
