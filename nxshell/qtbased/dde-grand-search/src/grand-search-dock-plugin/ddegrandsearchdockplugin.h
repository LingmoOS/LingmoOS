// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDEGRANDSEARCHDOCKPLUGIN_H
#define DDEGRANDSEARCHDOCKPLUGIN_H

#include "versiondefine.h"

#ifdef USE_DOCK_2_0
#    include <pluginsiteminterface_v2.h>
#else
#    include <pluginsiteminterface.h>
#endif

#include <QObject>
#include <QScopedPointer>
#include <QLabel>

class QGSettings;

namespace GrandSearch {

class TipsWidget;
class GrandSearchWidget;
#ifdef USE_DOCK_2_0
class DdeGrandSearchDockPlugin : public QObject, public PluginsItemInterfaceV2
{
    Q_OBJECT
    // 声明实现了的接口
    Q_INTERFACES(PluginsItemInterfaceV2)
    // 插件元数据
    Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "ddegrandsearch_dockplugin2.json")
#else
class DdeGrandSearchDockPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    // 声明实现了的接口
    Q_INTERFACES(PluginsItemInterface)
    // 插件元数据
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "ddegrandsearch_dockplugin.json")
#endif
public:
    explicit DdeGrandSearchDockPlugin(QObject *parent = nullptr);
    ~DdeGrandSearchDockPlugin() override;

    // 返回插件的名称，必须是唯一值，不可以和其它插件冲突
    const QString pluginName() const override;

    // 返回插件展示名称
    const QString pluginDisplayName() const override;

    // 插件初始化函数
    void init(PluginProxyInterface *proxyInter) override;

    // 返回插件的 widget
    QWidget *itemWidget(const QString &itemKey) override;

    // 返回插件的 tipsWidget
    QWidget *itemTipsWidget(const QString &itemKey) override;

    // 返回插件是否允许被禁用
    bool pluginIsAllowDisable() override;

    // 返回插件当前是否处于被禁用状态
    bool pluginIsDisable() override;

    // 当插件的禁用状态被用户改变时此接口被调用
    void pluginStateSwitched() override;

    // 返回鼠标左键点击插件主控件后要执行的命令数据（鼠标释放时调用）
    const QString itemCommand(const QString &itemKey) override;

    // 返回插件主控件的排序位置
    int itemSortKey(const QString &itemKey) override;

    // 重新设置主控件新的排序位置（用户拖动了插件控件后）
    void setSortKey(const QString &itemKey, const int order) override;

    // 右键菜单
    const QString itemContextMenu(const QString &itemKey) override;

    // 菜单执行项
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

#ifdef USE_DOCK_2_0
    Dock::PluginFlags flags() const override
    {
        return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal;
    }
    virtual void setMessageCallback(MessageCallbackFunc cb) override { m_messageCallback = cb; }
#endif

private slots:
    void onGsettingsChanged(const QString &key);
    void onVisibleChanged(bool visible);

private:
    QScopedPointer<GrandSearchWidget> m_searchWidget;
    QScopedPointer<QGSettings> m_gsettings;
    QScopedPointer<TipsWidget> m_tipsWidget;
    QScopedPointer<QWidget> m_quickWidget;

#ifdef USE_DOCK_2_0
    MessageCallbackFunc m_messageCallback = nullptr;
#endif
};

}

#endif   // DDEGRANDSEARCHDOCKPLUGIN_H
