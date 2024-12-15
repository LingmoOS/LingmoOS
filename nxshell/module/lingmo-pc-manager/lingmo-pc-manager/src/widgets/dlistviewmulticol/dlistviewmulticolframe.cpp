// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dlistviewmulticolframe.h"

#include <DApplicationHelper>

#include <QDebug>
#include <QVBoxLayout>

DListViewMultiColFrame::DListViewMultiColFrame(QStandardItemModel *model, QWidget *parent)
    : DFrame(parent)
    , m_model(model)
    , m_listView(nullptr)
{
    initUi();
    initConnection();
}

DListViewMultiColFrame::~DListViewMultiColFrame() { }

DListViewMultiCol *DListViewMultiColFrame::listViewMultiCol()
{
    return m_listView;
}

void DListViewMultiColFrame::initUi()
{
    setContentsMargins(1, 1, 1, 1);
    setLineWidth(0);

    setBackgroundRole(DPalette::ColorType::ItemBackground);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    m_listView = new DListViewMultiCol(m_model, this);
    QFont f = m_listView->font();
    f.setPixelSize(14);
    m_listView->setFont(f);
    m_listView->setAccessibleName("listViewFrame_listView");
    mainLayout->addWidget(m_listView);
}

void DListViewMultiColFrame::initConnection()
{
    connect(m_listView,
            &DListViewMultiCol::headViewSortIndicatorChanged,
            this,
            &DListViewMultiColFrame::headViewSortIndicatorChanged);
}
