// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "syncitemwidget.h"

#include <DListView>

#include <QVBoxLayout>

SyncItemWidget::SyncItemWidget(QWidget *parent) : QWidget(parent),
    m_syncConfigView(new DListView(this))
    , m_syncItemView(new DListView(this))
{
    InitUI();
    connect(m_syncConfigView, &QListView::clicked, this, &SyncItemWidget::configClicked);
    connect(m_syncItemView, &QListView::clicked, this, &SyncItemWidget::itemClicked);
}

void SyncItemWidget::SetEnable(bool bEnable)
{
    m_syncConfigView->setEnabled(bEnable);
    m_syncItemView->setEnabled(bEnable);
}

void SyncItemWidget::UpdateConfigIndex(const QModelIndex &index)
{
    m_syncConfigView->update(index);
}

void SyncItemWidget::UpdateItemIndex(const QModelIndex &index)
{
    m_syncItemView->update(index);
}

void SyncItemWidget::SetViewModel(QStandardItemModel *configModel, QStandardItemModel *itemModel)
{
    m_syncConfigView->setModel(configModel);
    m_syncItemView->setModel(itemModel);
}

void SyncItemWidget::InitUI()
{
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setContentsMargins(0, 0, 0, 0);
    mainlayout->setSpacing(10);
    //同步系统配置
    m_syncConfigView->setBackgroundType(DStyledItemDelegate::BackgroundType::ClipCornerBackground);
    m_syncConfigView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_syncConfigView->setSelectionMode(QListView::SelectionMode::NoSelection);
    m_syncConfigView->setEditTriggers(DListView::NoEditTriggers);
    m_syncConfigView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_syncConfigView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_syncConfigView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_syncConfigView->setFrameShape(DListView::NoFrame);
    m_syncConfigView->setItemSpacing(1);
    m_syncConfigView->setViewportMargins(0, 0, 1, 0);
    //m_syncConfigView->setModel(m_configModel);
    m_syncConfigView->setIconSize(QSize(16, 16));

    //同步项
    m_syncItemView->setBackgroundType(DStyledItemDelegate::BackgroundType::RoundedBackground);
    m_syncItemView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_syncItemView->setSelectionMode(QListView::SelectionMode::NoSelection);
    m_syncItemView->setEditTriggers(DListView::NoEditTriggers);
    m_syncItemView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_syncItemView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_syncItemView->setFrameShape(DListView::NoFrame);
    m_syncItemView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_syncItemView->setItemSpacing(10);
    m_syncItemView->setViewportMargins(0, 0, 1, 0);
    //m_syncItemView->setModel(m_itemModel);
    m_syncItemView->setIconSize(QSize(24, 24));
    //
    mainlayout->addWidget(m_syncConfigView);
    mainlayout->addWidget(m_syncItemView);
    mainlayout->addStretch();
    setLayout(mainlayout);
}
