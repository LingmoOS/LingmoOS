// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef PLUGINSITEMINTERFACE_V2_H
#define PLUGINSITEMINTERFACE_V2_H

#include "pluginsiteminterface.h"

class PluginsItemInterfaceV2;
/**
 * @brief 消息回调函数
 * @param const QString & 发送给任务栏的数据
 * @param void * 任务栏回传指针
 *
 * @return 任务栏返回的数据
 * @since 2.0.0
 *
 * 传入和传出的数据都是json格式的字符串，具体协议见README
 *
 */
using MessageCallbackFunc = QString (*)(PluginsItemInterfaceV2 *, const QString&);

class PluginsItemInterfaceV2 : public PluginsItemInterface
{
public:
    /**
     * @brief 插件标识
     *
     * @return 详见PluginFlags的说明
     *
     * @since 2.0.0
     */
    virtual Dock::PluginFlags flags() const { return Dock::Type_System | Dock::Attribute_Normal; }

    /**
     * @brief 获取 icon
     * @param Dock::IconType 详见Dock::IconType枚举说明
     * @param Dock::ThemeType 详见Dock::ThemeType枚举说明
     * @return 图标对象
     *
     * @since 2.0.0
     */
    virtual QIcon icon(Dock::IconType, Dock::ThemeType) const { return QIcon(); }

    /**
    * @brief 给某个插件传递它要承载的插件指针
    * 主要给托盘插件和快捷面板插件使用，普通插件无需关注此接口
    * @param QPluginLoader* 插件指针
    *
    * @since 2.0.0
    */
    virtual void addPlugin(QPluginLoader*) {};

    /**
     * @brief 设置消息回调函数，插件可以通过此回调函数向任务栏发送请求
     * 主要是为了扩展性，并在扩展功能时不添加虚函数破坏二进制兼容
     * @param MessageCallbackFunc 消息回调指针
     * @param void * 任务栏回传指针
     *
     * @since 2.0.0
     */
    virtual void setMessageCallback(MessageCallbackFunc) {}

    /**
     * @brief 任务栏向插件发送请求，例如：获取数据、执行指令等
     * 主要是为了扩展性，并在扩展功能时不添加虚函数破坏二进制兼容
     * 使用json格式字符串传输数据，具体协议见README
     *
     * @since 2.0.0
     */
    virtual QString message(const QString &) { return "{}"; }
};

QT_BEGIN_NAMESPACE

#define ModuleInterface_iid_V2 "com.deepin.dock.PluginsItemInterface_V2"

Q_DECLARE_INTERFACE(PluginsItemInterfaceV2, ModuleInterface_iid_V2)
QT_END_NAMESPACE

#endif