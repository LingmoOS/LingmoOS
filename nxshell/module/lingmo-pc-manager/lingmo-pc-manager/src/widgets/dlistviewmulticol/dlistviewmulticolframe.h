// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dlistviewmulticol.h"

#include <DFrame>

DWIDGET_USE_NAMESPACE

class DListViewMultiColFrame : public DFrame
{
    Q_OBJECT
public:
    explicit DListViewMultiColFrame(QStandardItemModel *model, QWidget *parent = nullptr);
    ~DListViewMultiColFrame() override;

    DListViewMultiCol *listViewMultiCol();

Q_SIGNALS:
    void headViewSortIndicatorChanged(int index, Qt::SortOrder order);

private:
    void initUi();
    void initConnection();

private:
    QStandardItemModel *m_model;
    DListViewMultiCol *m_listView;
};
