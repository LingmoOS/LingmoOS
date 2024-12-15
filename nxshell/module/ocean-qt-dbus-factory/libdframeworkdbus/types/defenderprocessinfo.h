// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERPROCESSINFO_H
#define DEFENDERPROCESSINFO_H

#include <QObject>
#include <QList>
#include <QString>
#include <QtDBus>
#include <QDBusMetaType>

struct DefenderProcessInfo
{
    int proc;
    QString title;
    QString desktop;

    bool operator!=(const DefenderProcessInfo &procInfo);
};
QDBusArgument &operator<<(QDBusArgument &argument, const DefenderProcessInfo &procInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderProcessInfo &procInfo);

typedef QList<DefenderProcessInfo> DefenderProcessInfoList;
Q_DECLARE_METATYPE(DefenderProcessInfo)
Q_DECLARE_METATYPE(DefenderProcessInfoList)

void registerDefenderProcessInfoMetaType();

#endif // DEFENDERPROCESSINFO_H
