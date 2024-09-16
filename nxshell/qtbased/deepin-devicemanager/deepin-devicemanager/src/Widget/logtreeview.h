// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGTREEVIEW_H
#define LOGTREEVIEW_H

#include <DTreeView>
#include <QKeyEvent>
#include <QStandardItem>
#include "logviewheaderview.h"
#include "logviewitemdelegate.h"

class LogTreeView : public Dtk::Widget::DTreeView
{
    Q_OBJECT
public:
    explicit LogTreeView(QWidget *parent = nullptr);

    /**
     * @brief setHeaderLabels : 设置table的表头
     * @param lst ：表头的内容
     */
    void setHeaderLabels(const QStringList &lst);

    /**
     * @brief setItem : 设置表格的item
     * @param row : item设置到哪一行
     * @param column : item设置到哪一列
     * @param item ：需要设置的item
     */
    void setItem(int row, int column, QStandardItem *item);

    /**
     * @brief item 获取表格的item
     * @param row 哪一行
     * @param column 哪一列
     * @return
     */
    QStandardItem *item(int row, int column);

    /**
     * @brief setColumnAverage : 设置表头等宽
     */
    void setColumnAverage();

    /**
     * @brief currentRowEnable : 获取当前行是否是选中状态
     * @return : 返回状态
     */
    bool currentRowEnable();

    /**
     * @brief currentRowAvailable : 获取当前行是否不可用状态
     * @return
     */
    bool currentRowAvailable();

    /**
     * @brief currentRow
     * @return
     */
    int currentRow();

    /**
     * @brief updateCurItemEnable : 判断当前行是否是被禁用状态
     * @param row
     * @param enable
     */
    void updateCurItemEnable(int row, int enable);

    /**
     * @brief clear : 清空数据
     */
    void clear();

    /**
     * @brief setRowNum: 设置表格总行数
     * @param row: 行数
     */
    void setRowNum(int row);

    /**
     * @brief RowNum:获取表格行数
     * @return  行数
     */
    int RowNum()const;

protected:
    /**
     * @brief initUI : Initialize the control layout
     */
    void initUI();
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int           m_RowCount;          // 表格行数

    QStandardItemModel         *mp_Model;
    LogViewItemDelegate        *mp_ItemDelegate;
    LogViewHeaderView          *mp_HeaderView;

};

#endif  // LOGTREEVIEW_H
