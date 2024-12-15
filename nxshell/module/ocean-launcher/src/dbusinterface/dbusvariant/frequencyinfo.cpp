// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "frequencyinfo.h"

FrequencyInfo::FrequencyInfo()
{

}

FrequencyInfo::~FrequencyInfo()
{

}

void FrequencyInfo::registerMetaType()
{
    qRegisterMetaType<FrequencyInfo>("FrequencyInfo");
    qDBusRegisterMetaType<FrequencyInfo>();
    qRegisterMetaType<FrequencyInfoList>("FrequencyInfoList");
    qDBusRegisterMetaType<FrequencyInfoList>();
}

QDebug operator<<(QDebug argument, const FrequencyInfo &info)
{
    argument << info.m_key << info.m_count;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const FrequencyInfo &info)
{
    argument.beginStructure();
    argument << info.m_key << info.m_count;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const FrequencyInfo &info)
{
    argument << info.m_key << info.m_count;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, FrequencyInfo &info)
{
    argument.beginStructure();
    argument >> info.m_key >> info.m_count;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, FrequencyInfo &info)
{
    argument >> info.m_key >> info.m_count;

    return argument;
}
