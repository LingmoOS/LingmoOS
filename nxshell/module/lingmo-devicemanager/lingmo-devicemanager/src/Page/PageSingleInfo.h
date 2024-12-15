// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGESINGLEINFO_H
#define PAGESINGLEINFO_H

#include <QObject>
#include <QWidget>

#include <DLabel>

#include "PageInfo.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

using namespace Dtk::Widget;

class PageTableWidget;
class RichTextDelegate;

/**
 * @brief The PageSingleInfo class
 * UI 单个设备的界面展示类
 */
class PageSingleInfo : public PageInfo
{
    Q_OBJECT
public:
    explicit PageSingleInfo(QWidget *parent = nullptr);
    ~PageSingleInfo() override;

    /**
     * @brief updateInfo:更新信息
     * @param lst:某类设备信息列表
     */
    virtual void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief clearWidgets clear widgets
     */
    void clearWidgets() override;

    /**
     * @brief setLabel:设置类型Label
     * @param itemstr:类型字符串
     */
    void setLabel(const QString &itemstr) override;

    /**
     * @brief loadDeviceInfo:加载设备信息
     * @param lst:某设备设备的信息列表
     */
    virtual void loadDeviceInfo(const QList<QPair<QString, QString>> &lst);

    /**
     * @brief clearContent:清除表格内容
     */
    void clearContent() override;

    /**
     * @brief isExpanded 判断是否展开
     * @return 是否展开
     */
    bool isExpanded();


    /**
     * @brief setRowHeight: 设置行高
     * @param row： 行
     * @param height：行高
     */
    void setRowHeight(int row, int height);

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

protected slots:
    /**
     * @brief slotShowMenu:鼠标右键菜单槽函数
     */
    void slotShowMenu(const QPoint &);

    /**
     * @brief slotActionCopy:右键菜单拷贝信息
     */
    void slotActionCopy();

    /**
     * @brief slotActionEnable:右键菜单禁用启用
     */
    void slotActionEnable();

    /**
     * @brief slotActionCopy:右键菜单更新驱动
     */
    void slotActionUpdateDriver();

    /**
     * @brief slotActionCopy:右键菜单卸载驱动
     */
    void slotActionRemoveDriver();

    /**
     * @brief slotWakeupMachine:设置可以唤醒机器
     */
    void slotWakeupMachine();

protected:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

    /**
     * @brief expandTable 手动张开表格
     */
    void expandTable();

protected:
    PageTableWidget         *mp_Content;
    DLabel                  *mp_Label;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QAction                 *mp_Copy;        //<! 拷贝
    QAction                 *mp_Enable;      //<! 启用/禁用
    QAction                 *mp_updateDriver;//<! 驱动更新
    QAction                 *mp_removeDriver;//<! 驱动卸载
    QAction                 *mp_WakeupMachine;//<! 唤醒设备
    QMenu                   *mp_Menu;        //<! 右键菜单

    DeviceBaseInfo          *mp_Device;      //<! 设备
    bool                    m_SameDevice;

    RichTextDelegate       *m_ItemDelegate;
};

#endif // PAGESINGLEINFO_H
