// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DFrame>
#include <DTableView>

#include <QHeaderView>
#include <QScrollBar>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

#define ROW_HEIGHT 30 // 初始化高度

class DefenderTableHeaderView;

namespace def {
namespace widgets {

class DefenderTable : public DFrame
{
    Q_OBJECT
public:
    explicit DefenderTable(QWidget *parent = nullptr, int height = ROW_HEIGHT);
    ~DefenderTable();

    // 初始化UI
    void initUI();

    // 设置相关联的表格和数据模型
    void setTableAndModel(DTableView *table, QStandardItemModel *model);

    // 设置行高
    void setRowHeight(int height);

    // 掩藏某列表头下标
    void setHeadViewSortHide(int nIndex);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    DefenderTableHeaderView *m_headerDelegate; // 表头代理类

    DFrame *m_mainBackgroundWidgt;    // 填充背景的frame控件
    QStandardItemModel *m_tableModel; // 数据模型
    DTableView *m_table;              // 表格
    QList<DFrame *> m_backgroundLst;  // 背景列表

    int m_rowHeight; // 行高
    int m_rowCount;  // 总共的行数
};

} // namespace widgets
} // namespace def
