// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"
#include "base/signal.h"

namespace Lingmo::timedate {
class TimeDate;
}

namespace settings {

// Network time, through timedated, and the status bar's 24-hour clock.
class TimeSettings
{
public:
    explicit TimeSettings(Lingmo::timedate::TimeDate& timeDate);

    bool useNtp() const { return m_useNtp; }
    // Shows at once; a refused or cancelled password puts it back.
    void setUseNtp(bool enabled);
    // Whether a network time service is installed.
    bool canNtp() const { return m_canNtp; }

    bool twentyFour() const { return m_twentyFour; }
    void setTwentyFour(bool enabled);

    Lingmo::base::Signal<> useNtpChanged;
    Lingmo::base::Signal<> twentyFourChanged;

private:
    Lingmo::timedate::TimeDate& m_timeDate;
    Lingmo::base::Subscription m_timeDateChanged;
    Lingmo::base::dbus::Proxy m_statusBar;
    bool m_canNtp;
    bool m_useNtp;
    bool m_twentyFour;
};

} // namespace settings
