// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/power/powersettings.h"

#include <algorithm>
#include <cstdint>
#include <string>

#include "base/paths.h"
#include "base/settings.h"

using namespace Lingmo::base;

namespace settings {

static int normalizedTimeout(int timeout)
{
    return timeout < 0 ? -1 : std::max(1, timeout);
}

static int readTimeout(const Settings& settings, const std::string& key, int fallback)
{
    return settings.contains(key) ? normalizedTimeout(int(settings.intValue(key, 0))) : fallback;
}

PowerSettings::PowerSettings()
    : m_service(dbus::Connection::session(), "org.lingmo.Services", "/org/lingmo/Services/Power",
                "org.lingmo.Services.Power")
{
    const Settings settings(paths::lingmoConfig("services"));
    m_batteryScreenOff = readTimeout(settings, "Power/BatteryScreenOff", kDefaultBatteryScreenOff);
    m_acScreenOff = readTimeout(settings, "Power/ACScreenOff", kDefaultACScreenOff);
    m_acPreventSleep = settings.boolValue("Power/ACPreventSleep", false);
}

void PowerSettings::setBatteryScreenOff(int timeout)
{
    timeout = normalizedTimeout(timeout);
    if (timeout == m_batteryScreenOff)
        return;

    m_batteryScreenOff = timeout;
    m_service.send("setBatteryScreenOff", int32_t(timeout));
}

void PowerSettings::setACScreenOff(int timeout)
{
    timeout = normalizedTimeout(timeout);
    if (timeout == m_acScreenOff)
        return;

    m_acScreenOff = timeout;
    m_service.send("setACScreenOff", int32_t(timeout));
}

void PowerSettings::setACPreventSleep(bool prevent)
{
    if (prevent == m_acPreventSleep)
        return;

    m_acPreventSleep = prevent;
    m_service.send("setACPreventSleep", prevent);
}

} // namespace settings
