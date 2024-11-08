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

#ifndef LINGMO_MENU_APP_GROUP_MODEL_H
#define LINGMO_MENU_APP_GROUP_MODEL_H

#include <QAbstractProxyModel>
#include "data-entity.h"

namespace LingmoUIMenu {

/**
 * @class AppGroupModel
 *
 * 根据app的group属性进行分组,将同一组的应用挂在到某一个索引下
 */
class AppGroupModel : public QAbstractProxyModel
{
    Q_OBJECT
//    Q_PROPERTY(LingmoUIMenu::DataEntity::PropertyName sourceModel READ sourceModel WRITE setSourceModel NOTIFY sourceModelChanged)
public:
    explicit AppGroupModel(QObject *parent = nullptr);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    bool hasChildren(const QModelIndex &parent) const override;

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &proxyIndex, int role) const override;

    Q_INVOKABLE int findLabelIndex(const QString &label) const;

private Q_SLOTS:
    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);

private:
    void rebuildAppGroups();
    void reLocationIndex(int base, int offset);
    int findGroupIndex(const QModelIndex &sourceIndex) const;
    void insertApp(int groupIndex, const QModelIndex &sourceIndex);

private:
    // 存储分组信息
    QVector<QVector<int>*> m_groups;
    bool m_needRebuild {false};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_GROUP_MODEL_H
