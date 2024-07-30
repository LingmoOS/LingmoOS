/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "usagecomputer.h"

#include <algorithm>

void UsageComputer::setTicks(long long system, long long user, long long wait, long long idle)
{
    // according to the documentation some counters can go backwards in some circumstances
    auto systemDiff = std::max(system - m_systemTicks, 0ll);
    auto userDiff = std::max(user - m_userTicks, 0ll);
    auto waitDiff = std::max(wait - m_waitTicks, 0ll);

    long long totalTicks = system + user + wait + idle;
    auto totalDiff = std::max(totalTicks - m_totalTicks, 0ll);

    auto percentage =  [totalDiff] (long long tickDiff) {
        if (tickDiff >= totalDiff) {
            return 100.0;
        }
        if (tickDiff > 0 && totalDiff > 0) {
            return 100.0 * tickDiff / totalDiff;
        }
        return 0.0;
    };

    systemUsage = percentage(systemDiff);
    userUsage = percentage(userDiff);
    waitUsage = percentage(waitDiff);
    totalUsage = percentage(systemDiff + userDiff + waitDiff);

    m_totalTicks = totalTicks;
    m_systemTicks = system;
    m_userTicks = user;
    m_waitTicks = wait;
}
