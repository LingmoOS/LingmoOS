// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windowinfomap.h"

#include <QDBusMetaType>

void registerWindowInfoMapMetaType()
{
    registerWindowInfoMetaType();

    qRegisterMetaType<WindowInfoMap>("WindowInfoMap");
    qDBusRegisterMetaType<WindowInfoMap>();
}

void registerWindowInfoMetaType()
{
    qRegisterMetaType<WindowInfo>("WindowInfo");
    qDBusRegisterMetaType<WindowInfo>();
}

QDebug operator<<(QDebug argument, const WindowInfo &info)
{
    argument << '(' << info.title << ',' << info.attention << ')';

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const WindowInfo &info)
{
    argument.beginStructure();
    argument << info.title << info.attention;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, WindowInfo &info)
{
    argument.beginStructure();
    argument >> info.title >> info.attention;
    argument.endStructure();

    return argument;
}

bool WindowInfo::operator==(const WindowInfo &rhs) const
{
    return attention == rhs.attention &&
           title == rhs.title;
}
