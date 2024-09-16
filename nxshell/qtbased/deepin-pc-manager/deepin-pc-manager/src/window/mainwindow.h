// Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "interface/frameproxyinterface.h"
#include "interface/moduleinterface.h"
#include "modules/common/common.h"
#include "utils.h"

#include <DDialog>
#include <DGuiApplicationHelper>
#include <DIconButton>
#include <DMainWindow>

#include <QCheckBox>
#include <QCloseEvent>
#include <QDBusContext>
#include <QPair>
#include <QStack>
#include <QStackedWidget>
#include <QSystemTrayIcon>

DWIDGET_BEGIN_NAMESPACE
class DListView;
class DBackgroundGroup;
class DDialog;
class DSettingsDialog;
class DFrame;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QHBoxLayout;
class QStandardItemModel;
class QGSettings;
QT_END_NAMESPACE

namespace def {
class SystemSettings;
}

DEF_NAMESPACE_BEGIN
class HomePageModule;
// 首页体检数据交互类
class HomePageModel;
class NatiavModule;
class SecurityToolsModule;
class AntiVirusWork;
class NetControlWidget;
class MultiSelectListView;
class RestartDefenderDialog;
class DBusInvokerInterface;
class SettingsInvokerInterface;
DEF_NAMESPACE_END

DWIDGET_USE_NAMESPACE

enum closeType { Tray, Exit, Ask, Count };

DEF_NAMESPACE_BEGIN

class MainWindow : public DTK_WIDGET_NAMESPACE::DMainWindow,
                   public FrameProxyInterface,
                   protected QDBusContext
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 模板函数,供各模块调用
    void popWidget(ModuleInterface *const inter) override; // 弹栈
    void popAndDelWidget(ModuleInterface *const inter) override;
    void pushWidget(ModuleInterface *const inter, QWidget *const w) override;         // 压栈
    void setModuleVisible(ModuleInterface *const inter, const bool visible) override; // 模块可视化
    void setCurrentModule(int iModuleIdx, int iPageIdx = 0) override; // 设置当前模块
    ModuleInterface *getCurrentModule() override;
    int getModuleIndex(const QString &name) override;      // 获得模块下标
    void setBackForwardButtonStatus(bool status) override; // 设置后退按钮可用状态

    // 显示默认设置弹框
    void showDefaultSettingDialog();
    // 显示用户反馈弹框
    void showUserReplyDialog();
    // 显示谈设置弹窗，并跳转到相应组位置
    void showSettingDialog(const QString &groupKey);
    void toggle();

    // 将窗口出显示栈, 并删除
    void popAndDelWidget();
    // 将窗口出显示栈
    void popWidget();
    void initAllModule();
    void showModulePage(const QString &module, const QString &page);
    int getModuleCount();

    void closeWindow();
    // 设置安装时间
    void setInstallTime();

protected:
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent *event) override;

Q_SIGNALS:
    void moduleVisibleChanged(const QString &module, bool visible);
    // 校验病毒库版本
    void notifyCheckVdbVersion();

private Q_SLOTS:
    void onFirstItemClick(const QModelIndex &index);
    void onTrayActivated(QSystemTrayIcon::ActivationReason state);
    void setSystemsafety(const QModelIndex &index, int sonindex);

    // 点击退回按钮槽
    void onBackWardClick(bool checked);
    // 更新后退按钮
    void updateBackwardBtn();

    // 主题类型改变
    void themeTypeChange(ColorType themeType);

private:
    // 初始化UI
    void initUI();
    // 初始化数据
    void initData();
    // 初始化信号槽
    void initSignalSlots();

    void initTray();
    void initCloseDialog(QCloseEvent *event);
    void modulePreInitialize();
    void popAllWidgets(int place = 0); // place is Remain count
    void pushNormalWidget(ModuleInterface *const inter,
                          QWidget *const w); // exchange third widget : push new widget

    // 主界面关闭方式配置项 获取/设置
    int getCloseType();
    void setCloseType(int value);

    // 主界面退出询问弹框配置项 获取/设置
    int getAskType();
    void setAskType(int value);

    // 寻找子页面索引
    int getPageIndex(QString pageName);

private:
    QHBoxLayout *m_contentLayout;
    DFrame *m_splitLine;
    QHBoxLayout *m_rightContentLayout;
    MultiSelectListView *m_navView;
    DTK_WIDGET_NAMESPACE::DBackgroundGroup *m_rightView;
    QStandardItemModel *m_navModel;
    QStack<QPair<ModuleInterface *, QWidget *>> m_contentStack;
    HomePageModule *m_homePageModule; // 首页体检模块
    HomePageModel *m_homePageModel;   // 首页体检数据处理对象
    SecurityToolsModule *m_securityToolsModule;
    QList<QPair<ModuleInterface *, QString>> m_modules;
    DIconButton *m_backwardBtn;     // 上一步
    QSystemTrayIcon *m_pSystemTray; // 系统托盘图标
    QMenu *m_trayMenu;              // 托盘目录
    DDialog *m_trayDialag;
    SettingsInvokerInterface *m_gsetting; // 安全中心gsetting配置
    DSettingsDialog *m_dsd;               // 设置弹框

    DGuiApplicationHelper *m_guiHelper; // 方便得到系统主题
    QStringList m_modulesDarakIconList; // 深色主题图标列表
    QStringList m_modulesLightIconList; // 浅色主题图标列表

    // 安全工具
    DBusInvokerInterface *m_pSecurityToolDBusInter; // 安全工具
};

DEF_NAMESPACE_END
