// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGETABLEWIDGET_H
#define PAGETABLEWIDGET_H

#include <QObject>
#include <QTableWidgetItem>

#include <DWidget>

DWIDGET_USE_NAMESPACE

class DetailTreeView;
class RichTextDelegate;

/**
 * @brief The PageTableWidget class
 * 表头控件
 */
class PageTableWidget : public DWidget
{
    Q_OBJECT

public:
    explicit PageTableWidget(DWidget *parent = nullptr);

    /**
     * @brief setLimitRow 限制行数
     * @param row 行数
     */
    void setLimitRow(int row);

    /**
     * @brief setColumnAndRow 设置行数和列数
     * @param row　行
     * @param column　列
     */
    void setColumnAndRow(int row, int column = 2);

    /**
     * @brief setItem 设置item
     * @param row　行
     * @param column　列
     * @param item 表格项
     */
    void setItem(int row, int column, QTableWidgetItem *item);

    /**
     * @brief toString 以字符串的方式获取信息
     * @return 单元格内容以字符串显示
     */
    QString toString();

    /**
     * @brief isOverview:是否是概况界面
     * @return false : 不是概况界面
     */
    virtual bool isOverview();

    /**
     * @brief isBaseBoard : 是否是主板信息界面
     * @return false : 不是主板信息界面
     */
    virtual bool isBaseBoard();

    /**
     * @brief setDeviceEnable 设置设备是否被禁用和是否可用
     * @param enable 是否被禁用，用户直接禁用
     * @param available 因为驱动导致的无法可用
     */
    void setDeviceEnable(bool enable, bool available);

    /**
     * @brief clear : 清空数据
     */
    void clear();

    /**
     * @brief setRowHeight 设置行高
     * @param row　指定哪一行
     * @param height　行高
     */
    void setRowHeight(int row, int height);

    /**
     * @brief setItemDelegateForRow 设置每一行的委托
     * @param row　指定行
     * @param itemDelegate　指定委托
     */
    void setItemDelegateForRow(int row, RichTextDelegate *itemDelegate);

    /**
     * @brief isCurDeviceEnable:当前设备是否启用
     * @return true:启用，false:禁用
     */
    bool isCurDeviceEnable();

    /**
     * @brief setCurDeviceState:设置当前设备状态
     * @param state true:启用;false:禁用
     */
    void setCurDeviceState(bool enable, bool available);

    /**
     * @brief expandTable 手动扩展表格
     */
    void expandTable();

    /**
     * @brief isExpanded 判断时候是扩展状态
     * @return 判断时候是扩展状态
     */
    bool isExpanded();

signals:
    /**
     * @brief enableDeviceSignal:禁用启用设备信号
     */
    void enableDeviceSignal();

protected:
    /**
     * @brief paintEvent:重绘事件
     * @param event:事件
     */
    void paintEvent(QPaintEvent *event) override;

private:
    /**
     * @brief initUI:初始化页面布局
     */
    void initUI();

private:
    DetailTreeView *mp_Table;
};

#endif // PAGETABLEWIDGET_H
