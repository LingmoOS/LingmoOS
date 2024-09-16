// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "iteminfo.h"
#include <QDebug>

void ItemInfo::registerMetaType()
{
    static bool registered = false;
    if (registered)
        return;

    registered = true;

    qRegisterMetaType<ItemInfo>("ItemInfo");
    qDBusRegisterMetaType<ItemInfo>();
    qRegisterMetaType<ItemInfoList>("ItemInfoList");
    qDBusRegisterMetaType<ItemInfoList>();
}

QDebug operator<<(QDebug argument, const ItemInfo &info)
{
    argument << info.m_desktop << info.m_name
             << info.m_key << info.m_iconKey
             << info.m_categoryId << info.m_installedTime;
#ifndef QT_DEBUG
    argument << info.m_keywords;
#endif

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo &info)
{
    argument.beginStructure();
    argument << info.m_desktop << info.m_name
             << info.m_key << info.m_iconKey
             << info.m_categoryId << info.m_installedTime;
#ifndef QT_DEBUG
    argument << info.m_keywords;
#endif

    argument.endStructure();

    return argument;
}

QDataStream &operator<<(QDataStream &argument, const ItemInfo &info)
{
    argument << info.m_desktop << info.m_name
             << info.m_key << info.m_iconKey
             << info.m_categoryId << info.m_installedTime;
#ifndef QT_DEBUG
    argument << info.m_keywords;
#endif

    return argument;
}

const QDataStream &operator>>(QDataStream &argument, ItemInfo &info)
{
    argument >> info.m_desktop >> info.m_name
            >> info.m_key >> info.m_iconKey
            >> info.m_categoryId >> info.m_installedTime;
#ifndef QT_DEBUG
    argument >> info.m_keywords;
#endif

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo &info)
{
    argument.beginStructure();
    argument >> info.m_desktop >> info.m_name
            >> info.m_key >> info.m_iconKey
            >> info.m_categoryId >> info.m_installedTime;
#ifndef QT_DEBUG
    argument >> info.m_keywords;
#endif
    argument.endStructure();

    return argument;
}
