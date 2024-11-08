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

#include "appwidgetmodel.h"
#include <QDebug>

AppWidgetModel::AppWidgetModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_roleName.insert(appWidgetItemQmlRole, "appQml");
    m_roleName.insert(appWidgetItemNameRole, "appName");
    appwidgetinterface = new AppWidget::KAppWidgetManager(this);
}

QVariant AppWidgetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // FIXME: Implement me!
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant();
}

int AppWidgetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    // FIXME: Implement me!
    return m_date.size();
}

QVariant AppWidgetModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    // FIXME: Implement me!
    switch (role) {
    case appWidgetItemQmlRole:
        return m_date.value(index.row()).appItemQml;
    case appWidgetItemNameRole:
        return m_date.value(index.row()).appItemName;
    default:
        break;
    }
    return QVariant();
}
QHash<int, QByteArray> AppWidgetModel::roleNames() const
{
    return m_roleName;
}
int AppWidgetModel::count() const
{
    return rowCount(QModelIndex());
}
void AppWidgetModel::insert(QString appItemQml, QString appItemName)
{
    int index = count();
    emit beginInsertRows(QModelIndex(), index, index);
    LIST_APPWIDGET_ITEM_INFO list;
    qDebug() << __FILE__ << __FUNCTION__ << ": appItemQml = " << appItemQml << ", itemname = " << appItemName;
    list.appItemQml = appwidgetinterface->getAppWidgetUiFile(appItemQml);
    list.appItemName = appItemName;
    m_date.insert(index, list);
    emit endInsertRows();
}
