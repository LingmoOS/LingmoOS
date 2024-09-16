// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATEGORYINFO_H
#define CATEGORYINFO_H

#include <QtDBus>
#include <QDebug>
#include <QDataStream>

class CategoryInfo
{
public:
    CategoryInfo();
    ~CategoryInfo();

    static void registerMetaType();

    friend QDebug operator<<(QDebug argument, const CategoryInfo &info);
    friend QDBusArgument &operator<<(QDBusArgument &argument, const CategoryInfo &info);
    friend QDataStream &operator<<(QDataStream &argument, const CategoryInfo &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, CategoryInfo &info);
    friend const QDataStream &operator>>(QDataStream &argument, CategoryInfo &info);

public:
    QString m_name;
    qlonglong m_id;
    QStringList m_items;
};

typedef QList<CategoryInfo> CategoryInfoList;

Q_DECLARE_METATYPE(CategoryInfo)
Q_DECLARE_METATYPE(CategoryInfoList)

#endif // CATEGORYINFO_H
