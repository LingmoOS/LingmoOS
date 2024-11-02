/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "servicesortfilterproxymodel.h"
#include "servicetablemodel.h"

#include <QCollator>
#include <QDebug>
#include <QLocale>

ServiceSortFilterProxyModel::ServiceSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

// 根据父索引判断是否还有更多的信息可获取
bool ServiceSortFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::canFetchMore(parent);
}

// 根据父索引获取更多的信息
void ServiceSortFilterProxyModel::fetchMore(const QModelIndex &parent)
{
    if (!filterRegExp().pattern().isEmpty()) {
        // 当搜索内容非空时为避免刷新问题，批量加载所有内容
        while (canFetchMore(parent)) {
            QSortFilterProxyModel::fetchMore(parent);
        }
    }

    if (canFetchMore(parent))
        QSortFilterProxyModel::fetchMore(parent);

    if (canFetchMore(parent))
        QSortFilterProxyModel::fetchMore(parent);
}

// 过滤指定行的索引信息是否匹配关键字
bool ServiceSortFilterProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    // 分别匹配服务名称、服务描述、主进程号
    QModelIndex index0 =
        sourceModel()->index(row, ServiceTableModel::ServiceNameColumn, parent);
    QModelIndex index1 =
        sourceModel()->index(row, ServiceTableModel::ServiceDescriptionColumn, parent);
    QModelIndex index2 =
        sourceModel()->index(row, ServiceTableModel::ServiceMainPIDColumn, parent);

    bool rc = false;
    if (index0.isValid())
        rc |= sourceModel()->data(index0).toString().contains(filterRegExp());
    if (index1.isValid())
        rc |= sourceModel()->data(index1).toString().contains(filterRegExp());
    if (index2.isValid())
        rc |= sourceModel()->data(index2).toString().contains(filterRegExp());

    return rc;
}

// 比较指定索引的两条记录
bool ServiceSortFilterProxyModel::lessThan(const QModelIndex &m_left, const QModelIndex &m_right) const
{
    const QModelIndex &left = m_right;
    const QModelIndex &right = m_left;
    switch (sortColumn()) {
    case ServiceTableModel::ServiceMainPIDColumn:
        return left.data().toUInt() < right.data().toUInt();
    case ServiceTableModel::ServiceNameColumn:
    case ServiceTableModel::ServiceDescriptionColumn: {
        return QString::compare(left.data(Qt::DisplayRole).toString(),
                                right.data(Qt::DisplayRole).toString()) < 0;
    }
    default:
        break;
    }
    return QSortFilterProxyModel::lessThan(left, right);
}
