// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONITORPLUGIN_H
#define MONITORPLUGIN_H

//  smo-plugin
#include <dde-dock/constants.h>
/* to void  //1070  and V23 exist same time */
#ifdef DOCK_API_VERSION
#if (DOCK_API_VERSION >= DOCK_API_VERSION_CHECK(2, 0, 0))
    #define  USE_API_QUICKPANEL20   //1070
#endif
#endif

#ifndef USE_API_QUICKPANEL20
#ifdef  DDE_DOCK_MAJORV6
    #define  DDE_DOCK_NEW_VERSION
#endif
#endif

#ifdef USE_API_QUICKPANEL20
#include <dde-dock/pluginsiteminterface_v2.h>
#include "quickpanelwidget.h"
#else
#include <pluginsiteminterface.h>
#endif
#include "monitorpluginbuttonwidget.h"

#include "dbus/dbusinterface.h"
#include "systemmonitortipswidget.h"

// Qt
#include <QDBusInterface>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QJsonDocument>
#include <DGuiApplicationHelper>
#include <QScopedPointer>

#ifdef USE_API_QUICKPANEL20
using namespace Dock;
#define DOCK_DEFAULT_POS   -1
#define PLUGIN_INTERDACE  PluginsItemInterfaceV2
#define PLUGIN_METADATA()  Q_PLUGIN_METADATA(IID ModuleInterface_iid_V2 FILE "system-monitor_20.json")
#else
#define DOCK_DEFAULT_POS   0
#define PLUGIN_INTERDACE  PluginsItemInterface
#define PLUGIN_METADATA()  Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "system-monitor.json")
#endif

class QGSettings;
//!
//! \brief The MonitorPlugin class
//! 实现插件接口对象的类
//!
class MonitorPlugin : public QObject, PLUGIN_INTERDACE
{
    Q_OBJECT
    Q_INTERFACES(PLUGIN_INTERDACE)
    PLUGIN_METADATA();

public:
    enum RateUnit {
        RateBit,
        RateByte,
        RateKb,
        RateMb,
        RateGb,
        RateTb,
        RateUnknow
    };
    Q_ENUM(RateUnit)

    //!
    //! \brief MonitorPlugin 构造函数
    //! \param parent 传递的父对象指针
    //!
    explicit MonitorPlugin(QObject *parent = Q_NULLPTR);
#ifdef USE_API_QUICKPANEL20
    //! \brief ~MonitorPlugin 析构函数
    ~MonitorPlugin();
#endif
    // 以下是必须要实现的PluginsItemInterface接口
    //!
    //! \brief pluginName 返回插件名称，用于在dde-dock内部管理插件时使用
    //! \return 插件名称
    //!
    const QString pluginName() const Q_DECL_OVERRIDE;
    //!
    //! \brief pluginDisplayName 返回插件名称，用于在界面上显示
    //! \return 返回插件名称
    //!
    const QString pluginDisplayName() const Q_DECL_OVERRIDE;
    //!
    //! \brief init 插件初始化入口函数
    //! \param proxyInter 主程序的进程
    //!
    void init(PluginProxyInterface *proxyInter) Q_DECL_OVERRIDE;
    //!
    //! \brief itemWidget 返回插件主控件，用于显示在dde-dock面板上
    //! \param itemKey 返回插件主控件
    //! \return
    //!
    QWidget *itemWidget(const QString &itemKey) Q_DECL_OVERRIDE;
    // 以上是必须要实现的PluginsItemInterface接口

    //!
    //! \brief pluginStateSwitched 当插件的禁用状态被用户改变时此接口被调用
    //!
    void pluginStateSwitched() Q_DECL_OVERRIDE;

#ifndef DDE_DOCK_NEW_VERSION
    //!
    //! \brief pluginIsAllowDisable 返回插件是否允许被禁用（默认不允许被禁用)
    //! \return
    //!
    bool pluginIsAllowDisable() Q_DECL_OVERRIDE { return true; }

    //!
    //! \brief pluginIsDisable 返回插件当前是否处于被禁用的状态
    //! \return
    //!
    bool pluginIsDisable() Q_DECL_OVERRIDE;
#endif
    //!
    //! \brief itemTipsWidget 返回鼠标悬浮在插件主控件上时显示的提示框控件
    //! \param itemKey
    //! \return 悬浮窗控件
    //!
    QWidget *itemTipsWidget(const QString &itemKey) Q_DECL_OVERRIDE;

    //!
    //! \brief itemCommand 返回鼠标左键点击插件主控件后要执行的命令数据
    //! \param itemKey
    //! \return
    //!
    const QString itemCommand(const QString &itemKey) Q_DECL_OVERRIDE;

    //!
    //! \brief displayModeChanged dde-dock显示模式发生改变时此接口被调用
    //! \param displayMode
    //!
    void displayModeChanged(const Dock::DisplayMode displayMode) Q_DECL_OVERRIDE;

    //!
    //! \brief itemSortKey 返回插件主控件的排序位置
    //! \param itemKey
    //! \return
    //!
    int itemSortKey(const QString &itemKey) Q_DECL_OVERRIDE;

    //!
    //! \brief setSortKey 重新设置主控件新的排序位置（用户拖动了插件控件后)
    //! \param itemKey
    //! \param order
    //!
    void setSortKey(const QString &itemKey, const int order) Q_DECL_OVERRIDE;

    //!
    //! \brief itemContextMenu 返回鼠标右键点击插件主控件后要显示的菜单数据
    //! \param itemKey
    //! \return
    //!
    const QString itemContextMenu(const QString &itemKey) Q_DECL_OVERRIDE;

    //!
    //! \brief invokedMenuItem 菜单项被点击后的回调函数
    //! \param itemKey
    //! \param menuId
    //! \param checked
    //!
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) Q_DECL_OVERRIDE;

#ifdef DDE_DOCK_NEW_VERSION
    ///
    /// the icon for the plugin
    /// themeType {0:UnknownType 1:LightType 2:DarkType}
    ///
    QIcon icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType()) Q_DECL_OVERRIDE;
#endif
#ifdef USE_API_QUICKPANEL20
    //!
    //! the flags for the plugin
    //!
    Dock::PluginFlags flags() const  { return Dock::Type_Quick | Dock::Quick_Panel_Single | Dock::Attribute_Normal; }
    //此接口只有在PluginsItemInterfaceV2下才可用
    virtual void setMessageCallback(MessageCallbackFunc cb) override { m_messageCallback = cb; }
#endif

private slots:
    //!
    //! \brief udpateInfo 更新CPU MEM NET信息
    //!
    void udpateInfo();

    //!
    //! \brief udpateTipsInfo 更新CPU MEM NET信息
    //!
    void udpateTipsInfo();

#ifdef USE_API_QUICKPANEL20
    //!
    //! \brief onClickQuickPanel mouse event active
    //!
    void onClickQuickPanel();

    //!
    //! \brief onSysMonPopVisibleChanged 系统监视器弹窗显示状态改变
    //!
    void onSysMonPopVisibleChanged(bool);
#endif

private:
    //!
    //! \brief loadPlugin 加载插件
    //!
    void loadPlugin();

    //!
    //! \brief calcCpuRate 计算CPU占用率
    //! \param totalCPU 总CPU占用率
    //! \param availableCPU 可用CPU占用率
    //!
    void calcCpuRate(qlonglong &totalCPU, qlonglong &availableCPU);


    //!
    //! \brief calcMemRate 计算内存mem
    //! \param memory 可用内存
    //! \param memoryAll 总内存
    //!
    void calcMemRate(qlonglong &memory, qlonglong &memoryAll);

    //!
    //! \brief calcNetRate 计算网络
    //! \param netDown 网络下载
    //! \param netUpload 网络上传
    //!
    void calcNetRate(qlonglong &netDown, qlonglong &netUpload);


    //!
    //! \brief setRateUnitSensitive 设置速率单位的大小写模式
    //! \param unit 传入进来的网速单位
    //! \param sensitive 设置速率单位大小写模式
    //! \return 速率单位字符串
    //!
    QString setRateUnitSensitive(RateUnit unit);
    //!
    //! \brief autoRateUnits 自动显示单位
    //! \param speed 传入的网速
    //! \param unit 智能调节后的网速的单位
    //! \return 自动调节单位后的速率
    //!
    double autoRateUnits(qlonglong speed, RateUnit &unit);

    //!
    //! \brief openSystemMonitor 打开系统监视器主界面
    //!
    void openSystemMonitor();

private:
#ifdef USE_API_QUICKPANEL20
    QuickPanelWidget *m_quickPanelWidget;
    //此类型(MessageCallbackFunc)属于PluginsItemInterfaceV2
    MessageCallbackFunc m_messageCallback;
#endif

    bool m_pluginLoaded;
    MonitorPluginButtonWidget *m_itemWidget = nullptr;
    QScopedPointer<SystemMonitorTipsWidget> m_dataTipsLabel;

    qlonglong m_down = 0;
    qlonglong m_upload = 0;
    qlonglong m_totalCPU = 0;
    qlonglong m_availableCPU = 0;

    QTimer *m_refershTimer;

    QString startup;

    QString m_cpuStr{"0.0"};       //转换后的cpu数据
    QString m_memStr{"0.0"};       //转换后的mem数据
    QString m_downloadStr{"0KB/s"};  //转换后的下载数据
    QString m_uploadStr{"0KB/s"};    //转换后的上传数据

};

#endif // MONITORPLUGIN_H
