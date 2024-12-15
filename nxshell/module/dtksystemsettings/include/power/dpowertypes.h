// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkpower_global.h"

#include <qobject.h>

DPOWER_BEGIN_NAMESPACE

struct History
{
    uint time;
    double value;
    uint state;
    friend bool operator==(const History &lhs, const History &rhs)
    {
        return lhs.time == rhs.time && lhs.value == rhs.value && lhs.state == rhs.state;
    }
};

struct Statistic
{
    double value;
    double accuracy;
    friend bool operator==(const Statistic &lhs, const Statistic &rhs)
    {
        return lhs.value == rhs.value && lhs.accuracy == rhs.accuracy;
    }
};

enum class KbdSource { Internal, External, Unknown };

enum class PowerMode { Performance, Balance, PowerSave, Unknown };

enum class LidClosedAction : qint32 { Suspend = 1, Hibernate, TurnoffScreen, DoNothing, Unknown };

enum class PowerBtnAction : qint32 {
    Shutdown = 0,
    Suspend,
    Hibernate,
    TurnoffScreen,
    DoNothing,
    Unknown
};

DPOWER_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_POWER_NAMESPACE::History)
Q_DECLARE_METATYPE(DTK_POWER_NAMESPACE::Statistic)
