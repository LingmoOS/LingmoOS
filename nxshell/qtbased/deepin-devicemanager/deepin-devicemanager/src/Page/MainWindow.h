// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "DBusInterface.h"
#include "DBusDriverInterface.h"

#include <DMainWindow>
#include <DStackedWidget>
#include <DButtonBox>
#include <qdbusconnection.h>
#include <QDBusInterface>
#include <QDBusReply>

#include <QObject>

class WaitingWidget;
class DeviceWidget;
class LoadInfoThread;
class PageDriverManager;
class DriverScanWidget;

using namespace Dtk::Widget;

/**
 * @brief The MainWindow class
 * 界面，主窗口
 */
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    /**
     * @brief refresh:界面刷新
     */
    void refresh();

    /**
     * @brief refreshBatteryStatus:刷新电池状态
     */
    void refreshBatteryStatus();

    /**
     * @brief exportTo:导出设备信息
     * @return true:导出成功，false:导出失败
     */
    bool exportTo();

    /**
     * @brief showDisplayShortcutsHelpDialog:显示快捷键帮主窗口
     */
    void showDisplayShortcutsHelpDialog();

    /**
     * @brief addJsonArrayItem:添加json键值对
     * @param windowJsonItems:保存json键值对数组
     * @param name:json名称
     * @param value:json值
     */
    void addJsonArrayItem(QJsonArray &windowJsonItems, const QString &name, const QString &value);

    /**
     * @brief getJsonDoc:获取json文本信息
     * @return json文本信息
     */
    void getJsonDoc(QJsonDocument &doc);

    /**
     * @brief windowMaximizing:窗口最大化
     */
    void windowMaximizing();

    /**
     * @brief swichStackWidget:切换到“驱动管理”页面
     */
    void swichStackWidget();

protected:
    /**
     * @brief: 事件的重写
     */
    virtual void resizeEvent(QResizeEvent *event)override;

    /**
     * @brief keyPressEvent:相应键盘按键事件
     * @param keyEvent:按键：E导出，F大小调整，F5刷新，C复制
     */
    void keyPressEvent(QKeyEvent *keyEvent) override;

    /**
     * @brief event:事件变化
     * @param event事件
     * @return 布尔
     */
    bool event(QEvent *event) override;

    /**
     * @brief closeEvent 重写关闭事件
     * @param event
     */
    void closeEvent(QCloseEvent *event) override;

private:
    /** @brief initWindow:对窗口进行一系列的初始化操作*/
    void initWindow();

    /** @brief initWindowSize:初始化窗口大小*/
    void initWindowSize();

    /** @brief 初始化标题栏 */
    void initWindowTitle();

    /** @brief initWidgets:初始化界面相关的内容*/
    void initWidgets();

    /**
     * @brief refreshDataBase:刷新设备信息
     */
    void refreshDataBase();
    /**
     * @brief refreshDataBaseLater:刷新设备信息
     */
    void refreshDataBaseLater();
private slots:
    /**
     * @brief slotSetPage
     * @param page
     */
    void slotSetPage(QString page);

    /**
     * @brief loadingFinishSlot:加载设备信息结束 槽
     * @param message:提示信息
     */
    void slotLoadingFinish(const QString &message);

    /**
     * @brief slotListItemClicked:ListView item点击槽函数
     * @param itemStr:item显示字符串
     */
    void slotListItemClicked(const QString &itemStr);

    /**
     * @brief slotRefreshInfo:刷新信息槽函数
     */
    void slotRefreshInfo();

    /**
     * @brief slotExportInfo:导出信息槽函数
     */
    void slotExportInfo();

    /**
     * @brief changeUI:UI界面变化,BIOS界面行高
     */
    void slotChangeUI();

signals:
    /**
     * @brief fontChange:系统字体变化
     */
    void fontChange();

private:
    /**@brief:主窗口的stackWidget，主要有两个widget，一个是等待界面，还有一个是信息显示界面*/
    DStackedWidget        *mp_MainStackWidget;
    WaitingWidget         *mp_WaitingWidget;           //加载界面
    DeviceWidget          *mp_DeviceWidget;            //设备显示界面
    DriverScanWidget      *mp_DriverScanWidget;        //驱动管理扫描界面
    PageDriverManager     *mp_DriverManager;           //驱动管理主界面
    LoadInfoThread        *mp_WorkingThread;           //信息加载线程
    DButtonBox            *mp_ButtonBox;               // titlebar上添加Buttonbox
    bool                  m_refreshing = false;        // 判断界面是否正在刷新
    bool                  m_IsFirstRefresh = true;
    bool                  m_ShowDriverPage = false;
    bool                  m_statusCursorIsWait = false;
};

#endif // MAINWINDOW_H
