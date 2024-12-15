// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "localelist.h"

QDBusArgument &operator<<(QDBusArgument &arg, const LocaleInfo &info)
{
    arg.beginStructure();
    arg << info.id << info.name;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, LocaleInfo &info)
{
    arg.beginStructure();
    arg >> info.id >> info.name;
    arg.endStructure();

    return arg;
}

QDataStream &operator<<(QDataStream &ds, const LocaleInfo &info)
{
    return ds << info.id << info.name;
}

const QDataStream &operator>>(QDataStream &ds, LocaleInfo &info)
{
    return ds >> info.id >> info.name;
}

bool LocaleInfo::operator ==(const LocaleInfo &info)
{
    return id==info.id && name==info.name;
}

void registerLocaleListMetaType()
{
    qRegisterMetaType<LocaleInfo>("LocaleInfo");
    qDBusRegisterMetaType<LocaleInfo>();

    qRegisterMetaType<LocaleList>("LocaleList");
    qDBusRegisterMetaType<LocaleList>();
}
