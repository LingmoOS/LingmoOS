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

#ifndef __SERVICE_SORT_FILTER_PROXY_MODEL_H__
#define __SERVICE_SORT_FILTER_PROXY_MODEL_H__

#include <QSortFilterProxyModel>

class ServiceSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ServiceSortFilterProxyModel(QObject *parent = nullptr);

    /**
     * @brief fetchMore 根据父索引获取更多的信息
     * @param parent 父索引
     */
    void fetchMore(const QModelIndex &parent) override;

    /**
     * @brief canFetchMore 根据父索引判断是否还有更多的信息可获取
     * @param parent 父索引
     * @return 返回true表示还可以获取更多信息，否则返回false为已获取全部信息
     */
    bool canFetchMore(const QModelIndex &parent) const override;

protected:
    /**
     * @brief filterAcceptsRow 过滤指定行的索引信息是否匹配关键字
     * @param row 被过滤的行
     * @param parent 行对应的索引
     * @return 返回true表示被过滤的行匹配到关键字，对应的索引会被包含在模型中，否则false
     */
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;

    /**
     * @brief lessThan 比较指定索引的两条记录
     * @param left 左比较值的索引
     * @param right 右比较值的索引
     * @return 返回true表示左值小于右值，否则false
     */
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
};

#endif  // __SERVICE_SORT_FILTER_PROXY_MODEL_H__
