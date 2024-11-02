/*
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef LINGMO_QUICK_PLAYER_ITEMS_MODEL_H
#define LINGMO_QUICK_PLAYER_ITEMS_MODEL_H
#include <QAbstractListModel>
#include "properties.h"
class MprisPlayerCollecter;
class PlayerItemsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static PlayerItemsModel *self();

    QHash<int, QByteArray> roleNames() const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    void operation(const QModelIndex &index, MprisProperties::Operations operation, const QVariantList &args);

private Q_SLOTS:
    void onPlayerAdded(const QString &service, uint pid);
    void onPlayerRemoved(const QString &service, uint pid);
    void onDataChanged(const QString &service, const QVector<int> &properties);

private:
    explicit PlayerItemsModel(QObject *parent = nullptr);
    QStringList m_services;
};


#endif //LINGMO_QUICK_PLAYER_ITEMS_MODEL_H
