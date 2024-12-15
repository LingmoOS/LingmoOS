// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkpower_global.h"

#include <qdbusargument.h>
#include <qdbusmetatype.h>
#include <qlist.h>

DPOWER_BEGIN_NAMESPACE

struct History_p
{
    uint time;
    double value;
    uint state;
    static void registerMetaType();
};

struct Statistic_p
{
    double value;
    double accuracy;
    static void registerMetaType();
};

QDBusArgument &operator<<(QDBusArgument &arg, const History_p &value);
const QDBusArgument &operator>>(const QDBusArgument &arg, History_p &value);

QDBusArgument &operator<<(QDBusArgument &arg, const Statistic_p &value);
const QDBusArgument &operator>>(const QDBusArgument &arg, Statistic_p &value);

DPOWER_END_NAMESPACE

Q_DECLARE_METATYPE(DTK_POWER_NAMESPACE::History_p)
Q_DECLARE_METATYPE(DTK_POWER_NAMESPACE::Statistic_p)
