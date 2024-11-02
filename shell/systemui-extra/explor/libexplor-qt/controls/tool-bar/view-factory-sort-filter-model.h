/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef VIEWFACTORYSORTFILTERMODEL_H
#define VIEWFACTORYSORTFILTERMODEL_H

#include <QObject>
#include "explor-core_global.h"

#include <QSortFilterProxyModel>

namespace Peony {

class PEONYCORESHARED_EXPORT ViewFactorySortFilterModel2 : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit ViewFactorySortFilterModel2(QObject *parent = nullptr);

    const QModelIndex getIndexFromViewId(const QString &viewId);
    const QString getHighestPriorityViewId(int zoom_level_hint);
    const QStringList supportViewIds();

    const QIcon iconFromViewId(const QString &viewId);
    const QString getViewDisplayNameFromId(const QString &viewId);

public Q_SLOTS:
    void setDirectoryUri(const QString &uri);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

}

#endif // VIEWFACTORYSORTFILTERMODEL_H
