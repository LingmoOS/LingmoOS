// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace settings {

// Brightness of one display, addressed by its DRM connector name ("card0-eDP-1").
class Brightness
{
public:
    Brightness();

    const std::string& display() const { return m_display; }
    void setDisplay(const std::string& display);

    bool available() const { return m_available; }
    // 1 to 100.
    int value() const { return m_value; }
    void setValue(int value);

    // available() or value() changed.
    Lingmo::base::Signal<> changed;

private:
    void refresh();
    void handleValueChanged(const std::string& display, int value);

    Lingmo::base::dbus::Proxy m_service;
    std::vector<Lingmo::base::dbus::Match> m_matches;
    std::string m_display;
    bool m_available = false;
    int m_value = 0;
};

} // namespace settings
