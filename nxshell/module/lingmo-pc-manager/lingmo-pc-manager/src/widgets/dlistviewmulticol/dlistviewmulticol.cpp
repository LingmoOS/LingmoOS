// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlistviewmulticol.h"

#include "dlistviewmulticolcommon.h"
#include "dlistviewmulticoldelegate.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QScrollBar>
#include <QTimer>

DListViewMultiCol::DListViewMultiCol(QStandardItemModel *model, QWidget *parent)
    : DListView(parent)
    , m_model(model)
    , m_headerWidget(new QWidget(this))
    , m_headerView(new DefenderTableHeaderView(Qt::Orientation::Horizontal, m_headerWidget))
{
    initUi();
    initConnection();
}

DListViewMultiCol::~DListViewMultiCol() { }

void DListViewMultiCol::setRowHeight(int height)
{
    m_model->setProperty(PROPERTY_KEY_ROW_HEIGHT, height);
}

void DListViewMultiCol::setItemIconSize(const QSize &size)
{
    m_model->setProperty(PROPERTY_KEY_ITEM_ICON_SIZE, size);
}

// 设置某列表头下标是否可见
void DListViewMultiCol::setHeadSortingIndicatorVisible(int col, bool visible)
{
    m_headerView->setSortingIndicatorVisible(col, visible);
}

// 调整表头各列宽度
void DListViewMultiCol::resizeHeaderSection(int logicalIndex, int size)
{
    // 使用定时触发，确保所有控件信号已经连接完成
    QTimer::singleShot(1, this, [this, logicalIndex, size] {
        m_headerView->resizeSection(logicalIndex, size);
    });
}

void DListViewMultiCol::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    // 调整表头位置
    m_headerView->move(m_headerWidget->pos().x() - horizontalScrollBar()->value(),
                       m_headerWidget->pos().y());

    // 调整表头高度
    m_headerView->setFixedHeight(m_headerView->sizeHint().height());

    // 更新表头各列宽度
    updateHeaderWidth();

    // 使滚动条适应界面
    updateGeometries();
}

void DListViewMultiCol::updateGeometries()
{
    // 使滚动条适应界面
    resizeContents(m_headerView->length(), contentsSize().height());
    DListView::updateGeometries();

    update();
}

// 当表头各列宽度改变
void DListViewMultiCol::updateHeaderWidth()
{
    // 更新表头各列宽度列表
    m_headerWidthList.clear();
    for (int i = 0; i < m_model->columnCount(); ++i) {
        m_headerWidthList.append(m_headerView->sectionSize(i));
    }

    // 调整表头宽度
    if (m_headerView->length() != m_headerView->width()) {
        m_headerView->setFixedWidth(m_headerView->length());
    }

    // 填充最后一列
    if (m_headerView->stretchLastSection()) {
        stretchLastSectionToRightest();
    }

    m_headerWidget->setFixedHeight(m_headerView->sizeHint().height());
    m_headerWidget->setFixedWidth(m_headerView->width());

    // 设置参数
    m_model->setProperty(PROPERTY_KEY_HEADER_SECTION_WIDTH_LIST,
                         QVariant::fromValue<QList<int>>(m_headerWidthList));

    // 使滚动条适应界面
    updateGeometries();
}

// 当水平滑动条变动
void DListViewMultiCol::onHorizontalScrollBarValueChanged(int value)
{
    m_headerView->move(-value, 0);
    m_model->setProperty(PROPERTY_KEY_HEADER_X_OFFSET, -value);
}

// 当排序改变
void DListViewMultiCol::onHeadViewSortIndicatorChanged(int index, Qt::SortOrder order)
{
    Q_EMIT headViewSortIndicatorChanged(index, order);
    // 使滚动条适应界面
    updateGeometries();
}

void DListViewMultiCol::initUi()
{
    setModel(m_model);
    // ListMode
    setViewMode(ViewMode::ListMode);
    setItemDelegate(new DListViewMultiColDelegate(this));
    setOrientation(QListView::TopToBottom, false);
    setTextElideMode(Qt::ElideMiddle);
    setSelectionRectVisible(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setUniformItemSizes(true);
    setResizeMode(Fixed);
    setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
    setEditTriggers(DListView::EditTrigger::NoEditTriggers);
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(false);

    // headerView
    // DListView顶部控件水平和数值位移固定，不受滑动条控制
    // 因此需要在顶部添加一个空控件，来适应实际表头的高
    addHeaderWidget(m_headerWidget);

    // set header view properties
    m_headerView->setParent(m_headerWidget); // 为响应表头拖动事件，需设置表头父对象为顶部空控件
    m_headerView->setModel(Q_NULLPTR);
    m_headerView->setModel(m_model);

    m_headerView->setStretchLastSection(true);
    m_headerView->setHighlightSections(false);
    m_headerView->setSectionsClickable(true);
    m_headerView->setSortIndicatorShown(true);
    m_headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // 表头各部分不可移动
    m_headerView->setSectionsMovable(false);
    m_headerView->setSectionResizeMode(QHeaderView::Interactive);

    updateHeaderWidth();
    setRowHeight(48);
    setItemIconSize(QSize(18, 18));
}

void DListViewMultiCol::initConnection()
{
    // header view connection
    // 表头尺寸改变
    connect(m_headerView, &QHeaderView::geometriesChanged, this, [this]() {
        m_headerWidget->setFixedHeight(m_headerView->sizeHint().height());
        m_headerWidget->setFixedWidth(m_headerView->length());
    });
    // 表头各列宽度改变
    connect(m_headerView, &QHeaderView::sectionResized, this, [this] {
        updateHeaderWidth();
    });
    connect(m_headerView, &QHeaderView::sectionCountChanged, this, [this] {
        updateHeaderWidth();
    });
    // 水平滑动条变动
    connect(horizontalScrollBar(),
            &QScrollBar::valueChanged,
            this,
            &DListViewMultiCol::onHorizontalScrollBarValueChanged);
    // 排序改变
    connect(m_headerView,
            &QHeaderView::sortIndicatorChanged,
            this,
            &DListViewMultiCol::onHeadViewSortIndicatorChanged);
}

void DListViewMultiCol::stretchLastSectionToRightest()
{
    // 当表头内容宽度小于界面宽度时，拓展表头控件
    if (m_headerView->length() < width()) {
        m_headerView->setFixedWidth(width());

        // 更新表头各列宽度列表
        m_headerWidthList.clear();
        int rightestSectionWidth = m_headerView->width();
        for (int i = 0; i < m_model->columnCount() - 1; i++) {
            rightestSectionWidth -= m_headerView->sectionSize(i);
            m_headerWidthList.append(m_headerView->sectionSize(i));
        }
        if (rightestSectionWidth > 0) {
            m_headerWidthList.append(rightestSectionWidth);
        }
    }
}
