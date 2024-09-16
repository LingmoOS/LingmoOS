// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logtreeview.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DStyledItemDelegate>

#include <QLoggingCategory>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHeaderView>
#include <QScrollBar>
#include <QPainterPath>

#include "MacroDefinition.h"
#include "TableWidget.h"

DWIDGET_USE_NAMESPACE

LogTreeView::LogTreeView(QWidget *parent)
    : DTreeView(parent)
    , m_RowCount(4)
    , mp_Model(nullptr)
    , mp_ItemDelegate(nullptr)
    , mp_HeaderView(nullptr)
{
    initUI();
}

void LogTreeView::setHeaderLabels(const QStringList &lst)
{
    if (mp_Model) {
        mp_Model->setHorizontalHeaderLabels(lst);
    }
}

void LogTreeView::setItem(int row, int column, QStandardItem *item)
{
    if (mp_Model) {
        mp_Model->setItem(row, column, item);
    }
}

QStandardItem *LogTreeView::item(int row, int column)
{
    if(mp_Model && mp_Model->rowCount() > row && mp_Model->columnCount() > column){
        return mp_Model->item(row,column);
    }
    return nullptr;
}

void LogTreeView::setColumnAverage()
{
    if (!mp_HeaderView) {
        return;
    }
    // 设置每一行等宽
    int colCount = mp_HeaderView->count();
    if (colCount == 0)
        return;

    int avgColWidth = width() / colCount;
    for (int i = 0; i < colCount; i++) {
        setColumnWidth(i, avgColWidth);
    }
}

bool LogTreeView::currentRowEnable()
{
    QModelIndex index = currentIndex();
    int row = index.row();
    if (row < 0) {
        return false;
    }
    QStandardItem *item = mp_Model->item(row, 0);
    if (item) {
        QString str = item->text();
        if (str.startsWith("(" + tr("Disable") + ")")) {
            return false;
        }
    }
    return true;
}

bool LogTreeView::currentRowAvailable()
{
    QModelIndex index = currentIndex();
     int row = index.row();
     if (row < 0) {
         return false;
     }
     QStandardItem *item = mp_Model->item(row, 0);
     if (item) {
         QString str = item->text();
         if (str.startsWith("(" + tr("Unavailable") + ")")) {
             return false;
         }
     }
     return true;
}

int LogTreeView::currentRow()
{
    QModelIndex index = currentIndex();
    return index.row();
}

void LogTreeView::updateCurItemEnable(int row, int enable)
{
    QStandardItem *item = mp_Model->item(row, 0);
    if (item) {
        QString str = item->text();
        if (enable) {
            str.replace("(" + tr("Disable") + ")", "");
        } else {
            str = "(" + tr("Disable") + ")" + str;
        }

        item->setText(str);
    }
}

void LogTreeView::clear()
{
    if (mp_Model)
        mp_Model->clear();
}

void LogTreeView::setRowNum(int row)
{
    // 设置表格行数
    m_RowCount = row;

    // 行数改变,表格高度要随之改变，为保证treewidget横向滚动条与item不重叠，添加滚动条高度(bug52470)
    this->setFixedHeight(TREE_ROW_HEIGHT * (m_RowCount + 1) + HORSCROLL_WIDTH);
}

int LogTreeView::RowNum() const
{
    return m_RowCount;
}

void LogTreeView::initUI()
{
    // 模型
    mp_Model = new QStandardItemModel(this);
    setModel(mp_Model);

    // Item 代理
    mp_ItemDelegate = new LogViewItemDelegate(this);
    setItemDelegate(mp_ItemDelegate);

    // 表头
    mp_HeaderView = new LogViewHeaderView(Qt::Horizontal, this);
    setHeader(mp_HeaderView);

    // 设置不可编辑
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //隐藏根节点项前的图标（展开折叠图标）
    this->setRootIsDecorated(false);


    // this is setting is necessary,because scrollperpixel is default in dtk!!
    this->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);

    // 设置选择模式
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    this->header()->setStretchLastSection(true);

    // 水平右对齐
    this->header()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 设置的固定高度,包括内容与表头(+1)，为保证treewidget横向滚动条与item不重叠，添加滚动条高度
    this->header()->setFixedHeight(TREE_ROW_HEIGHT);
    this->setFixedHeight(TREE_ROW_HEIGHT * (m_RowCount + 1) + HORSCROLL_WIDTH);


    // Item 不可扩展
    setItemsExpandable(false);

    // 设置无边框
    setFrameStyle(QFrame::NoFrame);
    this->viewport()->setAutoFillBackground(false);

    setAlternatingRowColors(false);
    setAllColumnsShowFocus(false);
}

void LogTreeView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setOpacity(1);
    painter.setClipping(true);

    QWidget *wnd = DApplication::activeWindow();
    DPalette::ColorGroup cg;
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }

    auto *dAppHelper = DApplicationHelper::instance();
    auto palette = dAppHelper->applicationPalette();

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    QStyleOptionFrame option;
    initStyleOption(&option);

    // 计算绘制背景色有问题
    QRect rect = viewport()->rect();

    QPainterPath clipPath;

    // 填充背景色
    clipPath.addRoundedRect(rect, 8, 8);

    painter.fillPath(clipPath, bgBrush);
    painter.restore();

    DTreeView::paintEvent(event);
}

void LogTreeView::showEvent(QShowEvent *event)
{
    //在show之前平均分布表头 Bug-73605
    setColumnAverage();
    return QTreeView::showEvent(event);
}

void LogTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
#endif
    DPalette::ColorGroup cg;
    if (!(options.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
#endif
    }

    auto *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;
    // 圆角以及边距
    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &options);
    auto margin = style->pixelMetric(DStyle::PM_ContentsMargins, &options);

    // modify background color acorrding to UI designer
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    QBrush background;

    // 隔行变色
    if (!(index.row() & 1)) {
        background = palette.color(cg, DPalette::ItemBackground);
    } else {
        background = palette.color(cg, DPalette::Base);
    }

    // 选中状态背景色
    if (options.state & DStyle::State_Enabled) {
        if (selectionModel()->isSelected(index)) {
            background = palette.color(cg, DPalette::Highlight);
        }
    }

    // 绘制背景色
    QPainterPath path;
    QRect rowRect { options.rect.x() - header()->offset(),
                    options.rect.y() + 1,
                    header()->length() - header()->sectionPosition(0),
                    options.rect.height() - 2 };
    rowRect.setX(rowRect.x() + margin);
    rowRect.setWidth(rowRect.width() - margin);

    path.addRoundedRect(rowRect, radius, radius);
    painter->fillPath(path, background);

    painter->restore();

    QTreeView::drawRow(painter, options, index);
}

void LogTreeView::keyPressEvent(QKeyEvent *event)
{
    DTreeView::keyPressEvent(event);
    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
        emit clicked(this->currentIndex());
    }
}

void LogTreeView::resizeEvent(QResizeEvent *event)
{
    DTreeView::resizeEvent(event);
    setColumnAverage();
}
