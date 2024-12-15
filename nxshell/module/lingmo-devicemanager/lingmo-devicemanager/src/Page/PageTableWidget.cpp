// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageTableWidget.h"
#include "DetailTreeView.h"
#include "PageInfo.h"
#include "RichTextDelegate.h"

// Dtk头文件
#include <DApplicationHelper>
#include <DPalette>
#include <DApplication>

// Qt库文件
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QLoggingCategory>
#include <QHeaderView>

PageTableWidget::PageTableWidget(DWidget *parent)
    : DWidget(parent)
    , mp_Table(new DetailTreeView(parent))
{
    // 初始化UI
    initUI();
}

void PageTableWidget::setLimitRow(int row)
{
    // 设置限制行数
    mp_Table->setLimitRow(row);
}

void PageTableWidget::setColumnAndRow(int row, int column)
{
    // 设置行列数
    mp_Table->setColumnAndRow(row, column);
}

void PageTableWidget::setItem(int row, int column, QTableWidgetItem *item)
{
    // 设置Item
    mp_Table->setItem(row, column, item);
}

QString PageTableWidget::toString()
{
    // table 内容转为QString
    return mp_Table->toString();
}

bool PageTableWidget::isOverview()
{
    // 是否是概况界面
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isOverview();
}

bool PageTableWidget::isBaseBoard()
{
    // 是否是主板界面
    PageInfo *par = dynamic_cast<PageInfo *>(this->parent());
    return par->isBaseBoard();
}

void PageTableWidget::clear()
{
    // 清空Table内容
    mp_Table->clear();
}

void PageTableWidget::setRowHeight(int row, int height)
{
    // 设置行高
    mp_Table->setRowHeight(row, height);
}

void PageTableWidget::setItemDelegateForRow(int row, RichTextDelegate *itemDelegate)
{
    // 设置单元格代理
    if (itemDelegate == nullptr) {
        mp_Table->setItemDelegateForRow(row, mp_Table->itemDelegate());
    } else {
        mp_Table->setItemDelegateForRow(row, itemDelegate);
    }
}

bool PageTableWidget::isCurDeviceEnable()
{
    // 当前设备是否可用
    return mp_Table->isCurDeviceEnable();
}

void PageTableWidget::setCurDeviceState(bool enable, bool available)
{
    // 设置当前设备状态
    mp_Table->setCurDeviceState(enable, available);
}

void PageTableWidget::expandTable()
{
    // 表格展开
    if (mp_Table)
        mp_Table->expandCommandLinkClicked();
}

bool PageTableWidget::isExpanded()
{
    if (mp_Table)
        return mp_Table->isExpanded();
    return false;
}

void PageTableWidget::setDeviceEnable(bool enable, bool available)
{
    // 设置设备状态
    mp_Table->setCurDeviceState(enable, available);
}

void PageTableWidget::paintEvent(QPaintEvent *event)
{
    DWidget::paintEvent(event);
}

void PageTableWidget::initUI()
{
    // 初始化页面布局
    QVBoxLayout *whLayout = new QVBoxLayout();

    // set Contents margin 0,bug66137
    whLayout->setContentsMargins(0, 0, 0, 0);
    whLayout->addWidget(mp_Table);
    whLayout->addStretch();
    this->setLayout(whLayout);
}
