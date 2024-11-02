/*
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
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#ifndef APPWIDGETMODEL_H
#define APPWIDGETMODEL_H

#include <kappwidgetmanager.h>
#include <QAbstractListModel>

struct LIST_APPWIDGET_ITEM_INFO {
    QString appItemQml;
    QString appItemName;
};

class AppWidgetModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AppWidgetModel(QObject *parent = nullptr);

    enum LIST_APPWIDGET_ITEM_ROLE {
        appWidgetItemQmlRole = Qt::UserRole + 1,
        appWidgetItemNameRole,
    };
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add data:
    Q_INVOKABLE void insert(QString appItemQml, QString appItemName) ;
    int count() const;
    QHash<int, QByteArray> roleNames() const override;
private:
    QList<LIST_APPWIDGET_ITEM_INFO> m_date;
    QHash<int, QByteArray> m_roleName;
    AppWidget::KAppWidgetManager * appwidgetinterface = nullptr;
};

#endif // APPWIDGETMODEL_H
