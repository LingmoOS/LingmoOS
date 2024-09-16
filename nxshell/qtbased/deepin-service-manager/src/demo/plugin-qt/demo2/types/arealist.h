// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef AREALIST_HPP
#define AREALIST_HPP

// <实现一个复杂类型的dbus类型>
// 1，定义一个QDBusMetaType类型
// 1.1, 定义一个自定义类型, include <QDBusMetaType>,
// 使用Q_DECLARE_METATYPE、qRegisterMetaType、qDBusRegisterMetaType
// 1.2，当类型为列表，如本例AreaList，需要重载operator<<和operator>>
// 2, xml的<method>中需要用<annotation name="org.qtproject.QtDBus.QtTypeName.In1" value=<type>/>
// 2.1, 参数AreaList，
//      In1: 代表自定义类型的参数位置
//      <type>: 指定类型， 如这里指定为value="AreaList"
// 2.2, 原理
//      用annotation申明后，In1变量的类型会被替换为AreaList，不会生成代码时不会对类型进行检查。然后生成的代码自动包含头文件"types/<type>.h"，<type>为类型名的小写，如"types/arealist.h"
// 3，编译环境配置：用QDBusMetaType需要C++11、INCLUDE目录添加上述2.2的types所在目录

#include <QDBusMetaType>

struct MonitRect
{
    int x1;
    int y1;
    int x2;
    int y2;

    MonitRect() { }

    MonitRect(int p1, int p2, int p3, int p4)
        : x1(p1)
        , y1(p2)
        , x2(p3)
        , y2(p4)
    {
    }

    friend QDBusArgument &operator<<(QDBusArgument &argument, const MonitRect &info);
    friend const QDBusArgument &operator>>(const QDBusArgument &argument, MonitRect &info);
};

typedef QList<MonitRect> AreaList;
Q_DECLARE_METATYPE(MonitRect)
Q_DECLARE_METATYPE(AreaList)

void registerAreaListMetaType();

#endif // AREALIST_HPP
