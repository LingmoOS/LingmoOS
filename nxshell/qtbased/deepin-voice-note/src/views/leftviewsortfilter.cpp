// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "leftviewsortfilter.h"
#include "common/vnoteforlder.h"
#include "common/standarditemcommon.h"

/**
 * @brief LeftViewSortFilter::LeftViewSortFilter
 * @param parent
 */
LeftViewSortFilter::LeftViewSortFilter(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

/**
 * @brief LeftViewSortFilter::lessThan
 * @param source_left
 * @param source_right
 * @return true source_left小于source_right
 */
bool LeftViewSortFilter::lessThan(
    const QModelIndex &source_left,
    const QModelIndex &source_right) const
{
    StandardItemCommon::StandardItemType leftSourceType = StandardItemCommon::getStandardItemType(source_left);
    StandardItemCommon::StandardItemType rightSourceType = StandardItemCommon::getStandardItemType(source_right);

    if (leftSourceType == StandardItemCommon::NOTEPADITEM && rightSourceType == StandardItemCommon::NOTEPADITEM) {
        VNoteFolder *leftSource = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(source_left));

        VNoteFolder *rightSource = reinterpret_cast<VNoteFolder *>(
            StandardItemCommon::getStandardItemData(source_right));
        if (-1 != leftSource->sortNumber && -1 != rightSource->sortNumber) {
            return leftSource->sortNumber < rightSource->sortNumber;
        } else {
            return leftSource->createTime < rightSource->createTime;
        }
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

/**
 * @brief LeftViewSortFilter::filterAcceptsRow
 * @param source_row
 * @param source_parent
 * @return false不显示，ｔｒue显示
 */
bool LeftViewSortFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    VNoteFolder *data = reinterpret_cast<VNoteFolder *>(StandardItemCommon::getStandardItemData(index));
    for (auto &it : m_blackFolders) { //找到不显示项返回false
        if (it == data) {
            return false;
        }
    }
    return true;
}

/**
 * @brief LeftViewSortFilter::setBlackFolders
 * @param folders 不显示的项
 */
void LeftViewSortFilter::setBlackFolders(const QList<VNoteFolder *> &folders)
{
    m_blackFolders = folders;
    invalidateFilter();
}
