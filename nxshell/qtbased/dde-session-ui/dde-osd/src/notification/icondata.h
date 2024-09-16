// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONDATA_H
#define ICONDATA_H

#include <QDBusArgument>
/*!
 * \~chinese \class IconData
 * \~chinese \brief 描述Icon属性的数据结构
 */
class IconData
{
public:
    friend QDebug operator<<(QDebug arg, const IconData &data);
    friend QDBusArgument &operator<<(QDBusArgument &arg, const IconData &data);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, IconData &data);

public:
    int width;
    int height;
    int rowstride;
    bool alpha;
    int bit;
    int cannel;
    QByteArray array;
};

#endif // ICONDATA_H
