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

#ifndef LINGMO_MENU_RECENTLY_INSTALLED_MODEL_H
#define LINGMO_MENU_RECENTLY_INSTALLED_MODEL_H

#include "app-list-plugin.h"
#include <QSortFilterProxyModel>
class QTimer;

namespace LingmoUIMenu {

class RecentlyInstalledModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit RecentlyInstalledModel(QObject *parent = nullptr);
    bool event(QEvent *event) override;

    QVariant data(const QModelIndex &index, int role) const override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

private:
    QTimer *m_timer {nullptr};
};

} // LingmoUIMenu

//Q_DECLARE_METATYPE(LingmoUIMenu::RecentlyInstalledModel*)

#endif //LINGMO_MENU_RECENTLY_INSTALLED_MODEL_H
