/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#ifndef LINGMO_MENU_COMBINED_LIST_MODEL_H
#define LINGMO_MENU_COMBINED_LIST_MODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QPair>

namespace LingmoUIMenu {

/**
 * 由多个子model组合成的model
 *
 * 通过index设置或获取子model
 * CombinedListModel
 */
class CombinedListModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit CombinedListModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &proxyIndex, int role) const override;

    // 获取subModel的数量
    int subModelCount() const;

    // 获取index位置和对应的subModel
    const QAbstractItemModel *subModelAt(int index) const;
    int indexOfSubModel(QAbstractItemModel *subModel);

    /**
     * 插入一个subModel
     * @param subModel
     * @param index 插入的位置，默认放到最后
     */
    void insertSubModel(QAbstractItemModel *subModel, int index = -1);
    void removeSubModel(int index);
    void removeSubModel(QAbstractItemModel *subModel);

private:
    int offsetOfSubModel(const QAbstractItemModel *subModel) const;

private:
    QVector<QPair<QAbstractItemModel*, int> > m_subModels;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_COMBINED_LIST_MODEL_H
