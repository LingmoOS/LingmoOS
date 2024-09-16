// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTSTRING_H
#define INTSTRING_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct IntString
{
    qint32 state;
    QString description;

    bool operator!=(const IntString &intString);
};

Q_DECLARE_METATYPE(IntString)

QDBusArgument &operator<<(QDBusArgument &argument, const IntString &intString);
const QDBusArgument &operator>>(const QDBusArgument &argument, IntString &intString);

void registerIntStringMetaType();

#endif // !INTSTRING_H
