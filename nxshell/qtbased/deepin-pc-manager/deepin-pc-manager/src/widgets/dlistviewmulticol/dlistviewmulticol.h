// Copyright (C) 2019 ~ 2021 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../defendertableheaderview.h"

#include <DListView>

DWIDGET_USE_NAMESPACE

// class DFrame;
class QHeaderView;

class DListViewMultiCol : public DListView
{
    Q_OBJECT
public:
    explicit DListViewMultiCol(QStandardItemModel *model, QWidget *parent = nullptr);
    ~DListViewMultiCol() override;

    void setRowHeight(int height);
    void setItemIconSize(const QSize &size);
    // 设置某列表头下标是否可见
    void setHeadSortingIndicatorVisible(int col, bool visible);
    // 调整表头各列宽度
    void resizeHeaderSection(int logicalIndex, int size);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void updateGeometries() override;

Q_SIGNALS:
    void headViewSortIndicatorChanged(int index, Qt::SortOrder order);

public slots:
    // 当表头各列宽度改变
    void updateHeaderWidth();
    // 当水平滑动条变动
    void onHorizontalScrollBarValueChanged(int value);
    // 当排序改变
    void onHeadViewSortIndicatorChanged(int index, Qt::SortOrder order);

private:
    void initUi();
    void initConnection();
    void stretchLastSectionToRightest();

private:
    QStandardItemModel *m_model;
    QWidget *m_headerWidget;
    DefenderTableHeaderView *m_headerView;
    QList<int> m_headerWidthList; // 表头各列宽度列表
};
