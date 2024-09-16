// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STRINGSTRUCT_H
#define STRINGSTRUCT_H

#include <QDBusMetaType>
#include <QList>

struct StringStruct {
    QString str1;
    QString str2;
    QString str3;

    bool operator ==(const StringStruct &other);
};

typedef QList<StringStruct> StringStructList;

Q_DECLARE_METATYPE(StringStruct)
Q_DECLARE_METATYPE(StringStructList)

QDBusArgument &operator<<(QDBusArgument &arg, const StringStruct &s);
const QDBusArgument &operator>>(const QDBusArgument &arg, StringStruct &s);

void registerStringStructListMetaType();

#endif // STRINGSTRUCT_H
