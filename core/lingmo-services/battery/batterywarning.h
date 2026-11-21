// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "notification.h"

// Warns at the levels UPower.conf sets, before UPower's critical power action.
class BatteryWarning
{
public:
    // UpDeviceLevel; UPower keeps it at None unless the battery is discharging.
    enum Level { Unknown, None, Discharging, Low, Critical, Action };

    // UPower on `system`, the notification on `session`.
    BatteryWarning(Lingmo::Base::DBus::Connection *system, Lingmo::Base::DBus::Connection *session);
    ~BatteryWarning();

    BatteryWarning(const BatteryWarning &) = delete;
    BatteryWarning &operator=(const BatteryWarning &) = delete;

private:
    void onPropertiesChanged(const Lingmo::Base::DBus::VariantMap &changed,
                             const std::vector<std::string> &invalidated);
    void fetchProperties();
    void fetchCriticalAction();
    void update();
    void notify(uint32_t level);
    std::string body(uint32_t level) const;

    Lingmo::Base::DBus::Proxy m_upower;
    Lingmo::Base::DBus::Proxy m_device;
    Lingmo::Base::DBus::Match m_changed;
    Lingmo::Base::DBus::PendingCall m_propertiesCall;
    Lingmo::Base::DBus::PendingCall m_actionCall;
    Notification m_notification;
    Lingmo::Base::Subscription m_failed;
    Lingmo::Base::DBus::VariantMap m_properties;
    std::string m_criticalAction;
    bool m_criticalActionKnown = false;
    // GetCriticalAction is in flight; its reply runs update() again.
    bool m_fetchingAction = false;
    uint32_t m_shownLevel = None;
};

