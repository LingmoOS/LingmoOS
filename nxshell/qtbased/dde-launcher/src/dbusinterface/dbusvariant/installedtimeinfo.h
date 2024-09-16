// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INSTALLEDTIMEINFO_H
#define INSTALLEDTIMEINFO_H

#include <QtDBus>

class InstalledTimeInfo
{
public:
    InstalledTimeInfo();
    ~InstalledTimeInfo();

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const InstalledTimeInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const InstalledTimeInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const InstalledTimeInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, InstalledTimeInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, InstalledTimeInfo &info);

public:
    QString m_key;
    qlonglong m_installedTime;
};

typedef QList<InstalledTimeInfo> InstalledTimeInfoList;

Q_DECLARE_METATYPE(InstalledTimeInfo)
Q_DECLARE_METATYPE(InstalledTimeInfoList)

#endif // INSTALLEDTIMEINFO_H
