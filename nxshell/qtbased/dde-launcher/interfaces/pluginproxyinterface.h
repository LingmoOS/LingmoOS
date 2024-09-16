// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGINPROXYINTERFACE_H
#define PLUGINPROXYINTERFACE_H

#include "common.h"

class PluginInterface;

class PluginProxyInterface
{
public:
    ~PluginProxyInterface() {}

    /**
     * @brief itemAdd 新增一个应用
     * @param interface 插件基类指针
     * @param info 应用的信息
     */
    virtual void itemAdded(PluginInterface * const interface, const AppInfo &info) = 0;

    /**
     * @brief itemRemove 移除一个应用
     * @param interface 插件基类指针
     * @param info 应用的信息
     */
    virtual void itemRemoved(PluginInterface * const interface, const AppInfo &info) = 0;

    /** 更新应用的信息，以指定数据刷新其显示内容
     * @brief itemUpdate 更新一个应用
     * @param interface 插件基类指针
     * @param info 应用的信息
     */
    virtual void itemUpdated(PluginInterface * const interface, const AppInfo &info) = 0;
};

#endif
