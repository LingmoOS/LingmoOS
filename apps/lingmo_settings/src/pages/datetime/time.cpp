// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/datetime/time.h"

#include "base/paths.h"
#include "base/settings.h"
#include "lingmo/timedate/time_date.h"

using namespace Lingmo;
using namespace Lingmo::base;

namespace settings {

TimeSettings::TimeSettings(timedate::TimeDate& timeDate)
    : m_timeDate(timeDate)
    , m_statusBar(dbus::Connection::session(), "org.lingmo.Statusbar", "/Statusbar", "org.lingmo.Statusbar")
    , m_canNtp(timeDate.canNtp())
    , m_useNtp(timeDate.ntp())
{
    m_twentyFour = Settings(paths::lingmoConfig("shell")).boolValue("Statusbar/TwentyFourTime", false);
    m_timeDateChanged = m_timeDate.changed.connect([this] {
        if (m_timeDate.ntp() != m_useNtp) {
            m_useNtp = m_timeDate.ntp();
            useNtpChanged.emit();
        }
    });
}

void TimeSettings::setUseNtp(bool enabled)
{
    if (m_useNtp == enabled)
        return;
    m_useNtp = enabled;
    m_timeDate.setNtp(enabled, [this, enabled](const std::string& error) {
        // A cancelled or failed polkit prompt leaves timedated as it was.
        if (!error.empty() && m_useNtp == enabled) {
            m_useNtp = !enabled;
            useNtpChanged.emit();
        }
    });
    useNtpChanged.emit();
}

void TimeSettings::setTwentyFour(bool enabled)
{
    if (m_twentyFour == enabled)
        return;
    m_twentyFour = enabled;
    // The status bar keeps the setting; it writes shell.conf itself.
    m_statusBar.send("setTwentyFourTime", enabled);
    twentyFourChanged.emit();
}

} // namespace settings
