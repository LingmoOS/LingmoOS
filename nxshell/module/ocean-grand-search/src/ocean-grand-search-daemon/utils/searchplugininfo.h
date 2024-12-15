// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHPLUGININFO_H
#define SEARCHPLUGININFO_H

#include <QString>

namespace GrandSearch {

struct SearchPluginInfo
{
    enum Mode {Auto, Manual, Trigger};   //运行模式，auto为由本程序控制启动，Manual为由外部控制，Trigger为通过DBus激活。
    enum Priority {High = 0, Middle, Low};  //优先级

    QString name;       //插件名，推荐使用com.company.app.xxxx 的格式
    QString ifsVersion; //dbus接口协议
    QString from;       //conf文件
    QString exec;       //插件启动命令
    QString service;    //dbus服务
    QString address;    //dbus地址
    QString interface;  //dbus接口

    Mode mode = Manual ;    //默认为Manual
    Priority priority = Low;    //仅在mode为auto时有效
};

}

#endif // SEARCHPLUGININFO_H
