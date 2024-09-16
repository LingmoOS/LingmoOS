// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "upowertypes_p.h"

#include <qdbusargument.h>
#include <qdbusmetatype.h>
#include <qlist.h>

DPOWER_BEGIN_NAMESPACE
QDBusArgument &operator<<(QDBusArgument &arg, const History_p &value)
{
    arg.beginStructure();
    arg << value.time;
    arg << value.value;
    arg << value.state;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, History_p &value)
{
    arg.beginStructure();
    arg >> value.time;
    arg >> value.value;
    arg >> value.state;
    arg.endStructure();
    return arg;
}

QDBusArgument &operator<<(QDBusArgument &arg, const Statistic_p &value)
{
    arg.beginStructure();
    ;
    arg << value.value;
    arg << value.accuracy;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, Statistic_p &value)
{
    arg.beginStructure();
    arg >> value.value;
    arg >> value.accuracy;
    arg.endStructure();
    return arg;
}

void History_p::registerMetaType()
{
    qRegisterMetaType<History_p>("History_p");
    qDBusRegisterMetaType<History_p>();
    qDBusRegisterMetaType<QList<History_p>>();
}

void Statistic_p::registerMetaType()
{
    qRegisterMetaType<Statistic_p>("Statistic_p");
    qDBusRegisterMetaType<Statistic_p>();
    qDBusRegisterMetaType<QList<Statistic_p>>();
}

DPOWER_END_NAMESPACE