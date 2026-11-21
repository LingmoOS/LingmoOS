// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/settings.h"

// One UPower device, published on the session bus when it powers the computer.
class Battery
{
public:
    // The chargeState values of org.lingmo.Services.Battery.
    enum ChargeState { NoCharge, Charging, Discharging, FullyCharged };

    Battery(Lingmo::Base::DBus::Connection *system, const std::string &path,
            Lingmo::Base::Settings &settings);
    ~Battery();

    Battery(const Battery &) = delete;
    Battery &operator=(const Battery &) = delete;

    // A battery of the computer, not of a mouse, a phone or a UPS.
    bool powersComputer() const;

    // org.lingmo.Services.Battery at /org/lingmo/Services/Battery; one
    // battery at a time holds the path.
    bool publish(Lingmo::Base::DBus::Connection *session);

    // "2h 5m"; empty under a minute.
    static std::string formatDuration(int64_t seconds);

private:
    struct Reading {
        uint32_t type = 0;
        uint32_t state = 0;
        int percent = 0;
        int capacity = 0;
        double voltage = 0;
        int64_t timeToEmpty = 0;
        int64_t timeToFull = 0;
        bool thresholdEnabled = false;

        ChargeState chargeState() const;
        // Seconds to full while charging, to empty while discharging, else -1.
        int64_t remainingTime() const;
        std::string status() const;
        bool operator==(const Reading &) const = default;
    };

    Reading read() const;
    void update();
    std::string lastChargedTime() const;

    std::string m_path;
    Lingmo::Base::Settings &m_settings;
    Lingmo::Base::DBus::Proxy m_device;
    Lingmo::Base::DBus::Match m_propertiesChanged;
    Lingmo::Base::DBus::Match m_prepareForSleep;
    std::unique_ptr<Lingmo::Base::DBus::Object> m_object;
    Reading m_reading;

    int m_lastChargedPercent = 0;
    uint64_t m_lastChargedSecs = 0;
};

