// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDTIME_H
#define RECORDTIME_H

#include <QtDBus/QtDBus>
#include <dde-dock/pluginsiteminterface_v2.h>
#include "timewidget.h"
#include "dbusservice.h"

class RecordTimePlugin : public QObject, PluginsItemInterfaceV2
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "recordtime.json")

public:
    explicit RecordTimePlugin(QObject *parent = nullptr);
    ~RecordTimePlugin() override;
    /**
     * @brief pluginName:返回插件名称
     * @return
     */
    const QString pluginName() const override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    /**
     * @brief pluginDisplayName:返回插件名称，用于界面显示
     * @return
     */
    const QString pluginDisplayName() const override;
    Dock::PluginFlags flags() const override { return Dock::Type_System | Dock::Attribute_ForceDock | Dock::Attribute_Normal; }
    PluginSizePolicy pluginSizePolicy() const override { return PluginsItemInterface::Custom; }

    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    /**
     * @brief init:插件初始化入口函数
     * @param proxyInter:主程序进程，上下文指针
     */
    void init(PluginProxyInterface *proxyInter) override;
    //cppcheck误报：此函数从未被使用，其实这个函数由dde-dock框架调用
    /**
     * @brief pluginIsAllowDisable:返回插件是否允许被禁用
     * @return
     */
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;
    void pluginStateSwitched() override;
    /**
     * @brief itemWidget:返回插件主控件，用于dde-dock面板上显示
     * @param itemKey:控件名称
     * @return
     */
    QWidget *itemWidget(const QString &itemKey) override;

    void clear();
public slots:
    /**
     * @brief onStart:启动计时服务
     */
    void onStart();

    /**
     * @brief onStop:停止计时服务
     */
    void onStop();

    /**
     * @brief onRecording:正在录屏
     */
    void onRecording();

    /**
     * @brief onPause:暂停
     */
    void onPause();
private:
    /**
     * @brief refresh:绕过dde-dock 2020.12版本对插件的控件大小的限制
     */
    void refresh();

private:
    QTimer *m_timer;
    QPointer<TimeWidget> m_timeWidget;
    QPointer<DBusService> m_dBusService;
    bool m_bshow;

    int m_nextCount = 0;
    int m_count = 0;
    /**
     * @brief 此定时器的作用为每隔1秒检查下截图录屏是否还在运行中。
     * 避免截图录屏崩溃后导致本插件还在执行
     */
    QTimer *m_checkTimer;
};

#endif // RECORDTIME_H
