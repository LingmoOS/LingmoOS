// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDTIME_H
#define RECORDTIME_H

#include "iconwidget.h"
#include "quickpanelwidget.h"
#include "tipswidget.h"
#include <QtDBus/QtDBus>
#include <dde-dock/pluginsiteminterface.h>

class ShotStartPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.shotstart" FILE "shotstart.json")
    Q_CLASSINFO("D-Bus Interface", "com.deepin.ShotRecorder.PanelStatus")

public:
    explicit ShotStartPlugin(QObject *parent = nullptr);
    ~ShotStartPlugin() override;
    /**
     * @brief pluginName:返回插件名称
     * @return
     */
    const QString pluginName() const override;
    /**
     * @brief pluginDisplayName:返回插件名称，用于界面显示
     * @return
     */
    const QString pluginDisplayName() const override;
    /**
     * @brief init:插件初始化入口函数
     * @param proxyInter:主程序进程，上下文指针
     */
    void init(PluginProxyInterface *proxyInter) override;
    /**
     * @brief pluginIsAllowDisable:返回插件是否允许被禁用
     * @return
     */
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;
    void pluginStateSwitched() override;

    /**
     * Dock::Type_Quick=0x02           插件类型-快捷插件区;
     * Dock::Quick_Panel_Single=0x40   当插件类型为Common时,快捷插件区域只有一列的那种插件;
     * Dock::Attribute_Normal=0xe00    普通插件;
     */
    Q_PROPERTY(int pluginFlags READ pluginFlags)
    int pluginFlags() const { return 0x02 | 0x40 | 0xe00; }
    PluginSizePolicy pluginSizePolicy() const override { return PluginsItemInterface::Custom; }
    /**
     * @brief itemWidget:返回插件主控件，用于dde-dock面板上显示
     * @param itemKey:控件名称
     * @return
     */
    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;

    int itemSortKey(const QString &itemKey) override;
    void setSortKey(const QString &itemKey, const int order) override;

    // 鼠标单击执行命令
    const QString itemCommand(const QString &itemKey) override;

    // 右键菜单
    const QString itemContextMenu(const QString &itemKey) override;
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

public Q_SLOTS:
    Q_SCRIPTABLE bool onStart();
    Q_SCRIPTABLE void onStop();
    Q_SCRIPTABLE void onRecording();
    Q_SCRIPTABLE void onPause();

private:
    void onClickQuickPanel();

private:
    QScopedPointer<IconWidget> m_iconWidget;              // 托盘图标
    QScopedPointer<QuickPanelWidget> m_quickPanelWidget;  // 快捷面板
    QScopedPointer<TipsWidget> m_tipsWidget;              // 提示信息，快捷键
    /**
     * @brief m_isRecording true:正在录屏 false:未启动录屏
     */
    bool m_isRecording;
    /**
     * @brief 此定时器的作用为每隔1秒检查下截图录屏是否还在运行中。
     * 避免截图录屏崩溃后导致本插件还在执行
     */
    QTimer *m_checkTimer;
    /**
     * @brief m_nextCount 用来判断录屏是否正在进行中
     */
    int m_nextCount = 0;
    /**
     * @brief m_count 用来判断录屏是否正在进行中
     */
    int m_count = 0;

    /**
     * @brief m_bdockapiversion 兼容性适配，老版的dock不支持快捷面板
     */
    bool m_bDockQuickPanel;
};

#endif  // RECORDTIME_H
