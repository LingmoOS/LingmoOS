// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include "common.h"
#include "pluginproxyinterface.h"

#include <QtCore>
#include <QList>

class PluginInterface
{
public:

    virtual ~PluginInterface() {}

    /**
     * @brief init 初始化插件，保存proxyInter 到 m_proxyInterface
     * @param proxyInter
     */
    virtual void init(PluginProxyInterface *proxyInter) = 0;

    /**
     * @brief pluginName 插件名称
     * @return 返回插件名称
     */
    virtual const QString pluginName() const = 0;

    /*
     * @brief pluginDisplayName 插件显示在界面上的名称
     * @return 界面上显示的名称
     */
    virtual const QString pluginDisplayName() = 0;

    /**
     * @param searchText 搜索关键字
     * @return 返回搜索的应用列表
     */
    virtual AppInfoList search(const QString &searchText) = 0;

protected:
    PluginProxyInterface *m_proxyInterface;
};

QT_BEGIN_NAMESPACE
#define PluginInterface_IID "org.deepin.dde.launcher.PluginInterface"
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_IID)
QT_END_NAMESPACE

#endif
