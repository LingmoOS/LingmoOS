// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "utils.h"
#include "src/window/modules/common/comdata.h"
#include "window/modules/common/common.h"
#include "window/interface/frameproxyinterface.h"
#include "window/interface/moduleinterface.h"
#include "datainterface_interface.h"
#include "daemonservice_interface.h"
#include "monitornetflow_interface.h"

#include <DMainWindow>
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <QCloseEvent>
#include <QGSettings>
#include <DDialog>
#include <QStack>
#include <QPair>
#include <QDBusContext>
#include <QSystemTrayIcon>
#include <QStackedWidget>

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DBackgroundGroup;
class DDialog;
class DSettingsDialog;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QStandardItemModel;
class QGSettings;
QT_END_NAMESPACE

DEF_NAMESPACE_BEGIN
class HomePageModule;
// 首页体检数据交互类
class HomePageModel;
class SecurityToolsModule;
class NetControlWidget;
class SecurityLogDialog;
class MultiSelectListView;
class RestartDefenderDialog;
DEF_NAMESPACE_END

DWIDGET_USE_NAMESPACE

enum closeType {
    Tray,
    Exit,
    Ask,
    Count
};

// 数据服务 / 心跳服务 / 病毒查杀服务 dbus接口 服务名称
using DataInterFaceServer = com::deepin::defender::datainterface;
using DaemonService = com::deepin::defender::daemonservice;
using MonitorInterFaceServer = com::deepin::defender::MonitorNetFlow;

DEF_NAMESPACE_BEGIN
class MainWindow : public DTK_WIDGET_NAMESPACE::DMainWindow
    , public FrameProxyInterface
    , protected QDBusContext
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 模板函数,供各模块调用
    void popWidget(ModuleInterface *const inter) override; //弹栈
    void popAndDelWidget(ModuleInterface *const inter) override;
    void pushWidget(ModuleInterface *const inter, QWidget *const w) override; //压栈
    void setModuleVisible(ModuleInterface *const inter, const bool visible) override; //模块可视化
    void setCurrentModule(int iModuleIdx, int iPageIdx = 0) override; //设置当前模块
    ModuleInterface *getCurrentModule() override;
    int getModuleIndex(const QString &name) override; //获得模块下标

    // 显示默认设置弹框
    void showDefaultSettingDialog();
    // 显示用户反馈弹框
    void showUserReplyDialog();
    // 显示安全日志弹框
    void showSecurityLogDialog();
    // 显示谈设置弹窗，并跳转到相应组位置
    void showSettingDialog(const QString &groupKey);
    void toggle();

    // 将窗口出显示栈, 并删除
    void popAndDelWidget();
    // 将窗口出显示栈
    void popWidget();
    void initDaemonService();
    void initAllModule();
    void showModulePage(const QString &module, const QString &page);
    int getModuleCount();

    // 联网管控弹框
    void showFuncConnectNetControl(const QString &sPkgName);
    // 远程访问弹框
    void showFuncConnectRemControl(const QString &sPkgName);
    // 流量详情弹框
    void showFuncConnectDataUsage();

    void closeWindow();
    // 设置安装时间
    void setInstallTime();

protected:
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;

Q_SIGNALS:
    void moduleVisibleChanged(const QString &module, bool visible);
    // 自定义控件值大小改变
    void customerValueChangge(QString info);

private Q_SLOTS:
    void onFirstItemClick(const QModelIndex &index);
    void onTrayActivated(QSystemTrayIcon::ActivationReason state);
    void setSystemsafety(const QModelIndex &index, int sonindex);

    // 点击退回按钮槽
    void onBackWardClick(bool checked);
    // 更新后退按钮
    void updateBackwardBtn();

    // 远程访问设置为始终允许
    void setRemDefaultAllowStatus();

    // 主题类型改变
    void themeTypeChange(ColorType themeType);

private:
    void initTray();
    void initCloseDialog(QCloseEvent *event);
    void modulePreInitialize();
    void popAllWidgets(int place = 0); //place is Remain count
    void pushNormalWidget(ModuleInterface *const inter, QWidget *const w); //exchange third widget : push new widget

    // 主界面关闭方式配置项 获取/设置
    int getCloseType();
    void setCloseType(int value);

    // 主界面退出询问弹框配置项 获取/设置
    int getAskType();
    void setAskType(int value);

    // usb存储设备保护 获取/设置
    int getUsbStorageType();
    void setUsbStorageType(int value);

    // 远程访问默认状态配置项 获取/设置
    int getRemType();
    void setRemType(int value);

    // 寻找子页面索引
    int getPageIndex(QString pageName);

private:
    QHBoxLayout *m_contentLayout;
    QHBoxLayout *m_rightContentLayout;
    MultiSelectListView *m_navView;
    DTK_WIDGET_NAMESPACE::DBackgroundGroup *m_rightView;
    QStandardItemModel *m_navModel;
    QStack<QPair<ModuleInterface *, QWidget *>> m_contentStack;
    HomePageModule *m_homePageModule; // 首页体检模块
    HomePageModel *m_homePageModel; // 首页体检数据处理对象
    SecurityToolsModule *m_securityToolsModule;
    QList<QPair<ModuleInterface *, QString>> m_modules;
    DIconButton *m_backwardBtn; // 上一步
    QSystemTrayIcon *m_pSystemTray; // 系统托盘图标
    QMenu *m_trayMenu; // 托盘目录
    DDialog *m_trayDialag;
    QGSettings *m_gsetting; // 安全中心gsetting配置
    DSettingsDialog *m_dsd; // 设置弹框
    NetControlWidget *m_netControlWidget; // 联网管控弹框
    SecurityLogDialog *m_securityLogDialog; // 安全日志弹框

    DataInterFaceServer *m_dataInterFaceServer; // 联网管控接口
    DaemonService *m_daemonservice; // 心跳接口
    MonitorInterFaceServer *m_monitorInterFaceServer; // 流量监控dbus类
    DGuiApplicationHelper *m_guiHelper; // 方便得到系统主题
    QStringList m_modulesDarakIconList; // 深色主题图标列表
    QStringList m_modulesLightIconList; // 浅色主题图标列表
};
DEF_NAMESPACE_END
