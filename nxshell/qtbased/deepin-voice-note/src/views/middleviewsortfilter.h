// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MIDDLEVIEWSORTFILTER_H
#define MIDDLEVIEWSORTFILTER_H

#include <QSortFilterProxyModel>
//列表项排序
class MiddleViewSortFilter : public QSortFilterProxyModel
{
public:
    explicit MiddleViewSortFilter(QObject *parent = nullptr);

    enum sortFeild {
        title,
        createTime,
        modifyTime,
    };
    //执行排序
    void sortView(
        sortFeild feild = modifyTime,
        int column = 0,
        Qt::SortOrder order = Qt::DescendingOrder);

protected:
    //处理排序
    virtual bool lessThan(
        const QModelIndex &source_left,
        const QModelIndex &source_right) const override;

    sortFeild m_sortFeild {modifyTime};
};

#endif // MIDDLEVIEWSORTFILTER_H
