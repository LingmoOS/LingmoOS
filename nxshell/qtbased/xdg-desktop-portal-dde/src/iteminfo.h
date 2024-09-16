// SPDX-FileCopyrightText: 2021 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef ITEMINFO_H
#define ITEMINFO_H

#include <QtDBus>

class ItemInfo
{
public:
    static void registerMetaType();

    inline bool operator==(const ItemInfo &other) const { return m_desktop == other.m_desktop; }
    friend QDebug operator<<(QDebug argument, const ItemInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const ItemInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const ItemInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, ItemInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, ItemInfo &info);

public:
    QString m_desktop;              // 应用的绝对路径
    QString m_name;                 // 应用名称
    QString m_key;                  // 应用所对应的二进制名称
    QString m_iconKey;              // 图标文件名称
    qlonglong m_categoryId;         // 应用分类的id,每个分类的id值不同
    qlonglong m_installedTime;      // 安装时间
#ifndef QT_DEBUG
    QStringList m_keywords;         // 搜索关键字
#endif
};

typedef QList<ItemInfo> ItemInfoList;

Q_DECLARE_METATYPE(ItemInfo)
Q_DECLARE_METATYPE(ItemInfoList)

#endif // ITEMINFO_H
