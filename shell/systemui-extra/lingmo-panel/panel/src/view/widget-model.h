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

#ifndef LINGMO_PANEL_WIDGET_MODEL_H
#define LINGMO_PANEL_WIDGET_MODEL_H

#include <QSortFilterProxyModel>
#include <QStringList>
#include <QPair>
#include <QPersistentModelIndex>

namespace LingmoUIQuick{
class WidgetItem;
}

namespace LingmoUIPanel {

class WidgetSortModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QList<int> widgetOrder READ widgetOrder WRITE setWidgetOrder NOTIFY widgetOrderChanged)
public:
    explicit WidgetSortModel(QObject *parent = nullptr);

    QList<int> widgetOrder() const;
    void setWidgetOrder(const QList<int> &order);

    void setSourceModel(QAbstractItemModel *sourceModel) override;

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

Q_SIGNALS:
    void widgetOrderChanged();

private:
    QList<int> m_widgetOrder;
};

} // LingmoUIPanel

#endif //LINGMO_PANEL_WIDGET_MODEL_H
