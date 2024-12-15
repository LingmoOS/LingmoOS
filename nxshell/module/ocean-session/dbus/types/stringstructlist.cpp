// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stringstructlist.h"

bool StringStruct::operator ==(const StringStruct &other)
{
    return (str1 == other.str1) && (str2 == other.str2) && (str3 == other.str3);
}

QDBusArgument &operator<<(QDBusArgument &arg, const StringStruct &s)
{
    arg.beginStructure();
    arg << s.str1 << s.str2 << s.str3;
    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, StringStruct &s)
{
    arg.beginStructure();
    arg >> s.str1 >> s.str2 >> s.str3;
    arg.endStructure();

    return arg;
}

void registerStringStructListMetaType()
{
    qRegisterMetaType<StringStruct>("StringStruct");
    qDBusRegisterMetaType<StringStruct>();

    qRegisterMetaType<StringStructList>("StringStructList");
    qDBusRegisterMetaType<StringStructList>();
}
