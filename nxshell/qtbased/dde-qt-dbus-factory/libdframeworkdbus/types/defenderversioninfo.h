// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFENDERVERSIONINFO_H
#define DEFENDERVERSIONINFO_H

#include <QtCore/QString>
#include <QtDBus/QtDBus>
#include <QDBusMetaType>

struct DefenderVersionInfo
{
    QString version;
    QString description;
    quint64 time;

    bool operator!=(const DefenderVersionInfo &versionInfo);
};

Q_DECLARE_METATYPE(DefenderVersionInfo)

QDBusArgument &operator<<(QDBusArgument &argument, const DefenderVersionInfo &versionInfo);
const QDBusArgument &operator>>(const QDBusArgument &argument, DefenderVersionInfo &versionInfo);

void registerDefenderVersionInfoMetaType();

#endif // DEFENDERVERSIONINFO_H
