// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "installedtimeinfo.h"

InstalledTimeInfo::InstalledTimeInfo()
{

}

InstalledTimeInfo::~InstalledTimeInfo()
{

}

void InstalledTimeInfo::registerMetaType()
{
    qRegisterMetaType<InstalledTimeInfo>("InstalledTimeInfo");
    qDBusRegisterMetaType<InstalledTimeInfo>();
    qRegisterMetaType<InstalledTimeInfoList>("InstalledTimeInfoList");
    qDBusRegisterMetaType<InstalledTimeInfoList>();
}

QDebug operator<<(QDebug argument, const InstalledTimeInfo &info)
{
    argument << info.m_key << info.m_installedTime;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const InstalledTimeInfo &info)
{
    argument.beginStructure();
    argument << info.m_key << info.m_installedTime;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const InstalledTimeInfo &info)
{
    argument << info.m_key << info.m_installedTime;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, InstalledTimeInfo &info)
{
    argument.beginStructure();
    argument >> info.m_key >> info.m_installedTime;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, InstalledTimeInfo &info)
{
    argument >> info.m_key >> info.m_installedTime;

    return argument;
}
