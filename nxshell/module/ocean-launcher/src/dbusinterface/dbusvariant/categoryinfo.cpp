// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "categoryinfo.h"

CategoryInfo::CategoryInfo()
{

}

CategoryInfo::~CategoryInfo()
{

}

void CategoryInfo::registerMetaType()
{
    qRegisterMetaType<CategoryInfo>("CategoryInfo");
    qDBusRegisterMetaType<CategoryInfo>();
    qRegisterMetaType<CategoryInfoList>("CategoryInfoList");
    qDBusRegisterMetaType<CategoryInfoList>();
}

QDebug operator<<(QDebug argument, const CategoryInfo &info)
{
    argument << info.m_id << info.m_name;
    argument << info.m_items;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const CategoryInfo &info)
{
    argument.beginStructure();
    argument << info.m_name << info.m_id << info.m_items;
    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const CategoryInfo &info)
{
    argument << info.m_name << info.m_id << info.m_items;

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, CategoryInfo &info)
{
    argument.beginStructure();
    argument >> info.m_name >> info.m_id >> info.m_items;
    argument.endStructure();

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, CategoryInfo &info)
{
    argument >> info.m_name >> info.m_id >> info.m_items;

    return argument;
}
