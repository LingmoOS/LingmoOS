// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERFILEINFO_H
#define DEFENDERFILEINFO_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct DefenderFileInfo
{
    int type;
    QString description;

    bool operator!=(const DefenderFileInfo &versionInfo);
};

Q_DECLARE_METATYPE(DefenderFileInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderFileInfo &versionInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderFileInfo &versionInfo);

void registerDefenderFileInfoMetaType();

#endif // DEFENDERFILEINFO_H
