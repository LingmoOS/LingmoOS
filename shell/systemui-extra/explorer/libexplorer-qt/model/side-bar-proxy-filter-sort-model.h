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

#ifndef SIDEBARPROXYFILTERSORTMODEL_H
#define SIDEBARPROXYFILTERSORTMODEL_H

#include "explorer-core_global.h"
#include <QSortFilterProxyModel>
#include <QLocale>
#include <QCollator>
namespace Peony {

class SideBarAbstractItem;

class PEONYCORESHARED_EXPORT SideBarProxyFilterSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SideBarProxyFilterSortModel(QObject *parent = nullptr);
    SideBarAbstractItem *itemFromIndex(const QModelIndex &proxy_index);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    QLocale m_locale;
    QCollator m_comparer;

    bool m_show_hidden = false;
};

}

#endif // SIDEBARPROXYFILTERSORTMODEL_H
