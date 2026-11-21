// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/thread_pool.h"

namespace Lingmo::upower {
class BatteryStatus;
} // namespace Lingmo::upower

namespace Lingmo::Shell {

// The status bar's battery: the framework's BatteryStatus, the icon for its
// level, and whether the percentage shows (Statusbar/BatteryPercentage in
// shell.conf).
class Battery
{
public:
    // `path` is shell.conf.
    Battery(std::shared_ptr<upower::BatteryStatus> status, std::string path);
    // lingmo-services starts after the compositor and is not activatable:
    // each time it gets an owner the status is read anew from `make`.
    void followService(base::dbus::Connection *sessionBus,
                       std::function<std::shared_ptr<upower::BatteryStatus>()> make);
    ~Battery();

    const upower::BatteryStatus &status() const { return *m_status; }

    bool isAvailable() const;
    bool onBattery() const;
    int chargePercent() const;
    const std::string &statusString() const;

    // battery-level-<0..100 in tens>[-charging]-symbolic.svg, as the bar's
    // images are named.
    std::string iconSource() const;
    static std::string iconSource(int percent, bool onBattery);

    bool showPercentage() const { return m_showPercentage; }
    void setShowPercentage(bool enabled);

    base::Signal<> changed;
    base::Signal<> showPercentageChanged;

private:
    void subscribe();

    std::shared_ptr<upower::BatteryStatus> m_status;
    std::function<std::shared_ptr<upower::BatteryStatus>()> m_make;
    base::dbus::NameWatch m_owner;
    std::string m_path;
    bool m_showPercentage = false;
    bool m_showPercentageSet = false;
    base::Job m_load;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

