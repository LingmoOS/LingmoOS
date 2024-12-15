// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusimagelist.h"

QDBusArgument &operator<<(QDBusArgument &argument, const DBusImage &image)
{
    argument.beginStructure();
    argument << image.width << image.height << image.pixels;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusImage &image)
{
    argument.beginStructure();
    argument >> image.width >> image.height >> image.pixels;
    argument.endStructure();
    return argument;
}

void registerDBusImageListMetaType()
{
    qRegisterMetaType<DBusImage>("DBusImage");
    qDBusRegisterMetaType<DBusImage>();

    qRegisterMetaType<DBusImageList>("DBusImageList");
    qDBusRegisterMetaType<DBusImageList>();
}

bool operator ==(const DBusImage &a, const DBusImage &b)
{
    return a.width == b.width
            && a.height == b.height
            && a.pixels == b.pixels;
}

bool operator !=(const DBusImage &a, const DBusImage &b)
{
    return !(a == b);
}
