// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/settings.h"
#include "base/signal.h"

class Battery;

// What UPower knows of the power source and the lid, and the computer's batteries.
class PowerSupply
{
public:
    // UPower on `system`; the battery that holds the path is published on `session`.
    PowerSupply(Lingmo::Base::DBus::Connection *system, Lingmo::Base::DBus::Connection *session,
                Lingmo::Base::Settings &settings);
    ~PowerSupply();

    PowerSupply(const PowerSupply &) = delete;
    PowerSupply &operator=(const PowerSupply &) = delete;

    bool onBattery() const { return m_onBattery; }
    bool hasLid() const { return m_hasLid; }
    bool lidClosed() const { return m_lidClosed; }

    Lingmo::Base::Signal<> powerSourceChanged;
    Lingmo::Base::Signal<> lidChanged;

private:
    void readFlags(const Lingmo::Base::DBus::VariantMap &values);
    void addBattery(const std::string &path);
    void removeBattery(const std::string &path);
    void publishBattery();

    Lingmo::Base::DBus::Connection *m_system;
    Lingmo::Base::DBus::Connection *m_session;
    Lingmo::Base::Settings &m_settings;
    Lingmo::Base::DBus::Proxy m_upower;
    std::vector<Lingmo::Base::DBus::Match> m_matches;
    // By UPower device path.
    std::map<std::string, std::unique_ptr<Battery>> m_batteries;
    Battery *m_published = nullptr;
    bool m_onBattery = false;
    bool m_hasLid = false;
    bool m_lidClosed = false;
};

