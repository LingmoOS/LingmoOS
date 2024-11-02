/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DEVELSORTTYPEMENU_H
#define DEVELSORTTYPEMENU_H

#include <QMenu>

class SortTypeMenu : public QMenu
{
    Q_OBJECT
public:
    explicit SortTypeMenu(QWidget *parent = nullptr);

    void setOriginPathVisible(bool visible);

Q_SIGNALS:
    void switchSortTypeRequest(int type);
    void switchSortOrderRequest(Qt::SortOrder order);
    void globalSortingPolicyChanged();

public Q_SLOTS:
    void setSortType(int type);
    void setSortOrder(Qt::SortOrder order);

private:
    int m_sort_type = 0;
    Qt::SortOrder m_sort_order = Qt::AscendingOrder;

    QAction *m_origin_path = nullptr;
    QActionGroup *m_sort_types;
    QActionGroup *m_sort_orders;
};

#endif // DEVELSORTTYPEMENU_H
