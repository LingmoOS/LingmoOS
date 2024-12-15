// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "time_period.h"

namespace common {
namespace core {

TimePeriod::TimePeriod()
    : m_period(kNoPeriod)
    , m_interval({2, 0})
{
}

TimePeriod::TimePeriod(TimePeriod::time_period_t period, const timeval &interval)
    : m_period(period)
    , m_interval(interval)
{
}

TimePeriod::TimePeriod(const TimePeriod &rhs)
    : m_period(rhs.m_period)
    , m_interval(rhs.m_interval)
{
}

bool TimePeriod::operator==(const TimePeriod &rhs) const
{
    return m_period == rhs.m_period && timercmp(&m_interval, &rhs.m_interval, ==);
}

bool TimePeriod::operator!=(const TimePeriod &rhs) const
{
    return !operator==(rhs);
}

void TimePeriod::registerMetaType()
{
    qRegisterMetaType<TimePeriod>("TimePeriod");
}

size_t TimePeriod::ticks() const
{
    if (m_interval.tv_sec == 0 && m_interval.tv_usec == 0)
        return 0;

    if (m_period == kNoPeriod) {
        return 2;
    }

    auto v = m_period / (m_interval.tv_sec * 1000000 + m_interval.tv_usec);
    return v < 1 ? 1 : size_t(v);
}

} // namespace core
} // namespace common
