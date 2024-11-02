/*
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 *
 */
#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H
#include <QAbstractItemModel>
#include "search-result-manager.h"
#include "one-result.h"

#define NUM_LIMIT_SHOWN_DEFAULT 5

namespace LingmoUISearch {

class SearchResultModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SearchResultModel(const QString &plugin_id);
    ~SearchResultModel() override;
    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    const SearchPluginIface::ResultInfo & getInfo(const QModelIndex&);
    void setExpanded(const bool&);
    const bool &isExpanded();
    void refresh();

public Q_SLOTS:
    void appendInfo(const SearchPluginIface::ResultInfo &);
    void startSearch(const QString &);
    void stopSearchSlot();

Q_SIGNALS:
    void stopSearch();
    void itemListChanged(const int&);
    void sendBestListData(const QString &, const SearchPluginIface::ResultInfo&);

private:
    void initConnections();
    QVector<OneResult*> m_items;
    QString m_plugin_id;
    SearchResultManager * m_search_manager = nullptr;
    bool m_isExpanded = false;
    QTimer * m_timer;
};
}

#endif // SEARCHRESULTMODEL_H
