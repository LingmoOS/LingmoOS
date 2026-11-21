// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::upower {

// One sample of a device's history, as UPower records it.
struct HistorySample
{
    // Unix seconds.
    uint32_t time = 0;
    double value = 0;
    // The device's state when the sample was taken, e.g. 1 charging.
    uint32_t state = 0;
};

// A UPower device, such as a battery: its properties, read asynchronously
// and kept up to date from PropertiesChanged, its charge limit and its
// history.
class Device
{
public:
    enum class HistoryType {
        Rate,
        Charge,
    };

    // The device's object path, e.g. /org/freedesktop/UPower/devices/battery_BAT0.
    explicit Device(std::string path, base::dbus::Connection* bus = base::dbus::Connection::system());
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    const std::string& path() const { return m_device.path(); }
    // Whether the first read has answered.
    bool isReady() const { return m_ready; }

    // The properties as last read or announced; empty when UPower has none.
    const base::dbus::VariantMap& properties() const { return m_properties; }
    // The device's directory name in /sys/class/power_supply, e.g. "BAT0".
    std::string nativePath() const;
    // UPower's charge limit; UPower before 1.90 has none of it.
    bool chargeThresholdSupported() const;
    bool chargeThresholdEnabled() const;
    int chargeStartThreshold() const;
    int chargeEndThreshold() const;

    // Turns the charge limit on or off between the thresholds the system
    // sets. Polkit lets the active session through and asks anyone else.
    void enableChargeThreshold(bool enabled, std::function<void(const base::dbus::Error& error)> done = {});

    // The samples over the last `timespan` seconds, at most `resolution` of
    // them, newest first as UPower returns them; empty on an error.
    using HistoryCallback = std::function<void(const std::optional<std::vector<HistorySample>>& samples)>;
    [[nodiscard]] base::dbus::PendingCall history(HistoryType type, uint32_t timespan, uint32_t resolution,
                                                  HistoryCallback done) const;

    base::Signal<> readyChanged;
    // With the properties that changed; a read's answer reports all of them.
    base::Signal<const base::dbus::VariantMap&> propertiesChanged;

private:
    void load();
    void merge(const base::dbus::VariantMap& properties);

    base::dbus::Proxy m_device;
    base::dbus::Match m_propertiesMatch;
    base::dbus::PendingCall m_loadCall;
    base::dbus::PendingCall m_enableCall;
    base::dbus::VariantMap m_properties;
    bool m_ready = false;
};

// A whole number of any of D-Bus's integer types, or the fallback.
int64_t integerValue(const base::dbus::Variant& value, int64_t fallback = 0);

} // namespace Lingmo::upower
