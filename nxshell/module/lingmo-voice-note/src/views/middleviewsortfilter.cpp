// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "middleviewsortfilter.h"
#include "common/vnoteitem.h"
#include "common/standarditemcommon.h"

/**
 * @brief MiddleViewSortFilter::MiddleViewSortFilter
 * @param parent
 */
MiddleViewSortFilter::MiddleViewSortFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

/**
 * @brief MiddleViewSortFilter::sortView
 * @param feild 排序类型
 * @param column 排序列
 * @param order 排序方式
 */
void MiddleViewSortFilter::sortView(MiddleViewSortFilter::sortFeild feild,
                                    int column,
                                    Qt::SortOrder order)
{
    m_sortFeild = feild;

    sort(column, order);
}

/**
 * @brief MiddleViewSortFilter::lessThan
 * @param source_left
 * @param source_right
 * @return true source_left小于source_right
 */
bool MiddleViewSortFilter::lessThan(
    const QModelIndex &source_left,
    const QModelIndex &source_right) const
{
    VNoteItem *leftNote = reinterpret_cast<VNoteItem *>(
        StandardItemCommon::getStandardItemData(source_left));

    VNoteItem *rightNote = reinterpret_cast<VNoteItem *>(
        StandardItemCommon::getStandardItemData(source_right));

    if (nullptr != leftNote && nullptr != rightNote) {
        if (leftNote->isTop != rightNote->isTop) {
            return leftNote->isTop ? false : true;
        }
        switch (m_sortFeild) {
        case modifyTime:
            return (leftNote->modifyTime < rightNote->modifyTime);
        case createTime:
            return (leftNote->createTime < rightNote->createTime);
        case title:
            return (leftNote->noteTitle < rightNote->noteTitle);
        }
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
