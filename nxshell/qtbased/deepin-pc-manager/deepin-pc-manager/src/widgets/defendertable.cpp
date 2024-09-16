// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defendertable.h"

#include "defendertableheaderview.h"

#include <DApplicationHelper>
#include <DFrame>
#include <DLabel>

#include <QtDebug>

DWIDGET_USE_NAMESPACE

namespace def {
namespace widgets {

DefenderTable::DefenderTable(QWidget *parent, int height)
    : DFrame(parent)
    , m_headerDelegate(nullptr)
    , m_mainBackgroundWidgt(nullptr)
    , m_tableModel(nullptr)
    , m_table(nullptr)
    , m_rowCount(0)
{
    setAccessibleName("DefenderTable");
    // 初始化行高
    m_rowHeight = height;

    // 初始化表头
    m_headerDelegate = new DefenderTableHeaderView(Qt::Horizontal, this);
    m_headerDelegate->setAccessibleName(accessibleName() + "_tableHeaderView");

    // 初始化界面
    initUI();
}

DefenderTable::~DefenderTable() { }

void DefenderTable::initUI()
{
    // 添加垂直布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(2, 2, 2, 2);
    setLayout(mainLayout);

    // 创建主题风格背景
    m_mainBackgroundWidgt = new DFrame(this);
    m_mainBackgroundWidgt->setAccessibleName(accessibleName() + "_bgWidget");
    m_mainBackgroundWidgt->lower();
    m_mainBackgroundWidgt->resize(size());

    // 设置表头
    QStandardItem *standaritem0 = new QStandardItem("head1.");
    standaritem0->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QStandardItem *standaritem1 = new QStandardItem("head2");
    standaritem1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 初始化数据模型
    m_tableModel = new QStandardItemModel(this);
    m_tableModel->clear();
    m_tableModel->setHorizontalHeaderItem(0, standaritem0);
    m_tableModel->setHorizontalHeaderItem(1, standaritem1);

    // 初始化表格
    m_table = new DTableView(this);
    m_table->setAccessibleName(accessibleName() + "_tableView");
    m_table->horizontalHeader()->setAccessibleName(accessibleName() + "_tableHead");

    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setFrameShape(QTableView::NoFrame);
    m_table->setFocusPolicy(Qt::NoFocus);

    // 自动调整最后一列的宽度使它和表格的右边界对齐
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setHidden(true);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
    m_table->verticalHeader()->setDefaultSectionSize(m_rowHeight);

    // 设置内容
    m_table->setModel(m_tableModel);

    // 设置每一行的高度
    for (int i = 0; i < m_rowCount; i++)
        m_table->setRowHeight(i, m_rowHeight);

    // 设置表格内容部分背景颜色为透明
    QPalette pa = m_table->palette();
    pa.setBrush(QPalette::Base, Qt::transparent);
    m_table->viewport()->setPalette(pa);

    // 为表格添加背景
    for (int i = 0; i < m_rowCount; i++) {
        DFrame *bgmTmp = new DFrame(m_table);
        bgmTmp->setFixedHeight(m_rowHeight);
        bgmTmp->setLineWidth(0);
        bgmTmp->lower();
        if (0 == i % 2) {
            bgmTmp->setBackgroundRole(DPalette::ItemBackground);
        }
        m_backgroundLst.append(bgmTmp);
    }

    mainLayout->addWidget(m_table);

    // 数据模型行数移除信号槽
    connect(m_tableModel, &QStandardItemModel::rowsRemoved, this, [=] {
        m_backgroundLst.last()->deleteLater();
        m_backgroundLst.removeLast();
        m_rowCount--;
    });

    // 数据模型行数插入信号槽
    connect(m_tableModel, &QStandardItemModel::rowsInserted, this, [=] {
        m_rowCount++;
        DFrame *bgmTmp = new DFrame(m_table);
        bgmTmp->setFixedHeight(m_rowHeight);
        bgmTmp->setLineWidth(0);
        bgmTmp->lower();
        if (0 == m_rowCount % 2) {
            bgmTmp->setBackgroundRole(DPalette::ItemBackground);
        }
        m_backgroundLst.append(bgmTmp);
    });
}

// 掩藏某列表头下标
void DefenderTable::setHeadViewSortHide(int nIndex)
{
    m_headerDelegate->setHeadViewSortHide(nIndex);
}

// 设置相关联的表格和数据模型
void DefenderTable::setTableAndModel(DTableView *table, QStandardItemModel *model)
{
    m_table->setVisible(false);

    disconnect(m_table, nullptr, nullptr, nullptr);
    disconnect(m_tableModel);

    m_table->deleteLater();
    m_tableModel->deleteLater();
    // 清除表格背景
    for (int i = 0; i < m_rowCount; i++) {
        m_backgroundLst.last()->deleteLater();
        m_backgroundLst.removeLast();
    }

    // 设置table and model
    m_table = table;
    m_tableModel = model;
    m_rowCount = m_tableModel->rowCount();

    // 使用代理更换表头
    m_table->setHorizontalHeader(m_headerDelegate);

    // 表头属性设置
    m_headerDelegate->setSectionsMovable(true);
    m_headerDelegate->setSectionsClickable(true);
    m_headerDelegate->setSectionResizeMode(DHeaderView::Interactive);
    m_headerDelegate->setStretchLastSection(true);
    m_headerDelegate->setSortIndicatorShown(true);
    m_headerDelegate->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_headerDelegate->setContextMenuPolicy(Qt::CustomContextMenu);
    m_headerDelegate->setFocusPolicy(Qt::TabFocus);

    // 设置表格属性
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setFrameShape(QTableView::NoFrame);
    m_table->setFocusPolicy(Qt::NoFocus);
    m_table->setWordWrap(false);

    // 自动调整最后一列的宽度使它和表格的右边界对齐
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->verticalHeader()->setHidden(true);
    m_table->setShowGrid(false);
    m_table->setSortingEnabled(true);
    // 设置内容
    m_table->setModel(m_tableModel);

    // 设置每一行的高度
    for (int i = 0; i < m_rowCount; i++)
        m_table->setRowHeight(i, m_rowHeight);

    // 设置表格内容部分背景颜色为透明
    QPalette pa = m_table->palette();
    pa.setBrush(QPalette::Base, Qt::transparent);
    m_table->viewport()->setPalette(pa);

    // 为表格每行添加背景
    for (int i = 0; i < m_rowCount; i++) {
        DFrame *bgmTmp = new DFrame(m_table);
        bgmTmp->setFixedHeight(m_rowHeight);
        bgmTmp->setLineWidth(0);
        bgmTmp->lower();
        if (0 == i % 2) {
            bgmTmp->setBackgroundRole(DPalette::ItemBackground);
        }
        m_backgroundLst.append(bgmTmp);
    }

    // 添加到主布局
    layout()->addWidget(m_table);

    // 数据模型行数移除信号槽
    connect(m_tableModel, &QStandardItemModel::rowsRemoved, this, [=] {
        m_backgroundLst.last()->deleteLater();
        m_backgroundLst.removeLast();
        m_rowCount--;
    });

    // 数据模型行数插入信号槽
    connect(m_tableModel, &QStandardItemModel::rowsInserted, this, [=] {
        m_rowCount++;
        DFrame *bgmTmp = new DFrame(m_table);
        bgmTmp->setFixedHeight(m_rowHeight);
        bgmTmp->setLineWidth(0);
        bgmTmp->lower();
        bgmTmp->show();
        if (0 == m_rowCount % 2) {
            bgmTmp->setBackgroundRole(DPalette::ItemBackground);
        }
        m_backgroundLst.append(bgmTmp);
    });
}

// 废弃于2020.4.30
void DefenderTable::setRowHeight(int height)
{
    m_rowHeight = height;
    // 设置每一行的高度
    for (int i = 0; i < m_rowCount; i++)
        m_table->setRowHeight(i, m_rowHeight);
}

void DefenderTable::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    m_mainBackgroundWidgt->resize(size());

    // 设置每一个表格项frame的属性以及位置
    int initY = m_table->viewport()->y();
    for (int i = 0; i < m_rowCount; i++) {
        int rowHeight = m_table->rowHeight(i);
        m_table->setRowHeight(i, rowHeight);
        m_backgroundLst[i]->setFixedWidth(m_table->width()
                                          + m_table->horizontalScrollBar()->maximum() - 1);
        m_backgroundLst[i]->setFixedHeight(rowHeight);
        m_backgroundLst[i]->move(-m_table->horizontalScrollBar()->value(),
                                 initY + i * rowHeight - m_table->verticalScrollBar()->value());
    }
}

void DefenderTable::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    // 设置每一个表格项frame的属性以及位置
    int initY = m_table->viewport()->y();
    for (int i = 0; i < m_rowCount; i++) {
        int rowHeight = m_table->rowHeight(i);
        m_table->setRowHeight(i, rowHeight);
        m_backgroundLst[i]->setFixedWidth(m_table->width()
                                          + m_table->horizontalScrollBar()->maximum() - 1);
        m_backgroundLst[i]->setFixedHeight(rowHeight);
        m_backgroundLst[i]->move(-m_table->horizontalScrollBar()->value(),
                                 initY + i * rowHeight - m_table->verticalScrollBar()->value());
    }
}

} // namespace widgets
} // namespace def
