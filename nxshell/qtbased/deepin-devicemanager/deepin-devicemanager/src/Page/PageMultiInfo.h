// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEPAGE_H
#define DEVICEPAGE_H

#include <QObject>
#include <DWidget>
#include <DLabel>

#include "PageInfo.h"


using namespace Dtk::Widget;

class PageTableHeader;
class PageDetail;
class DeviceBaseInfo;

/**
 * @brief The PageMultiInfo class
 * 多个设备信息的展示类
 */
class PageMultiInfo : public PageInfo
{
    Q_OBJECT
public:
    explicit PageMultiInfo(QWidget *parent = nullptr);
    ~PageMultiInfo() override;

    /**
     * @brief updateInfo:更新信息
     * @param lst:某类设备信息列表
     */
    void updateInfo(const QList<DeviceBaseInfo *> &lst)override;

    /**
     * @brief setLabel:设置类型Label
     * @param itemstr:类型字符串
     */
    void setLabel(const QString &itemstr) override;

    /**
     * @brief clearWidgets clear widgets
     */
    void clearWidgets() override;

signals:
    /**
     * @brief refreshInfo:刷新信息信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息信号
     */
    void exportInfo();

    /**
     * @brief enableDevice
     * @param row
     * @param enable
     */
    void enableDevice(int row, bool enable);

    /**
     * @brief updateUI : update UI
     */
    void updateUI();

protected:
    void resizeEvent(QResizeEvent* e) override;

private slots:
    /**
     * @brief slotItemClicked:点击表格Item
     * @param row:点击行
     */
    void slotItemClicked(int row);

    /**
     * @brief slotEnableDevice
     * @param row
     * @param enable
     */
    void slotEnableDevice(int row, bool enable);

    /**
     * @brief slotWakeupMachine
     * @param row
     * @param wakeup
     */
    void slotWakeupMachine(int row, bool wakeup);

    /**
     * @brief slotActionCopy:右键菜单更新驱动
     */
    void slotActionUpdateDriver(int row);

    /**
     * @brief slotActionCopy:右键菜单卸载驱动
     */
    void slotActionRemoveDriver(int row);

    /**
     * @brief slotCheckPrinterStatus 判断dde-printer是否被安装
     */
    void slotCheckPrinterStatus(int row, bool &isPrinter, bool &isInstalled);

private:
    /**
     * @brief initWidgets : 初始化控件布局
     */
    void initWidgets();

    /**
     * @brief getTableListInfo 获取需要在表格中显示的数据
     * @param deviceList 显示在表格中的数据
     * @param menuList 显示的右键菜单数据
     */
    void getTableListInfo(const QList<DeviceBaseInfo *> &lst, QList<QStringList>& deviceList, QList<QStringList>& menuList);

private:
    DLabel                    *mp_Label;
    PageTableHeader           *mp_Table;       //<! 上面的表格
    PageDetail                *mp_Detail;      //<! 下面的详细内容
    QList<DeviceBaseInfo *>   m_lstDevice;     //<! 保存设备列表
    QList<QStringList>        m_deviceList;    //<! 设备界面表格信息
    QList<QStringList>        m_menuControlList; //<! 设备界面菜单信息
};

#endif // DEVICEPAGE_H
