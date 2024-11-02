/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 */
#ifndef BESTLISTMODEL_H
#define BESTLISTMODEL_H

#include <QAbstractItemModel>
#include "search-result-model.h"

#define NUM_LIMIT_SHOWN_DEFAULT 5

namespace LingmoUISearch {
class BestListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit BestListModel(QObject *parent = nullptr);
    ~BestListModel();
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    const SearchPluginIface::ResultInfo & getInfo(const QModelIndex&);
    const QString & getPluginInfo(const QModelIndex&);

    void setExpanded(const bool&);
    const bool &isExpanded();

public Q_SLOTS:
    void appendInfo(const QString &, const SearchPluginIface::ResultInfo &);
    void removeInfo(const QString &);
    void moveInfo(const QString &pluginName, int pos);
    void startSearch(const QString &);
    void stopSearchSlot();

Q_SIGNALS:
    void stopSearch();
    void itemListChanged(const int&);

private:
    QVector<OneResult *> m_items;
    QVector<QString> m_pluginIdList;
    QVector<QString> m_resourceTypes;
    bool m_isExpanded = false;
};
}
#endif // BESTLISTMODEL_H
