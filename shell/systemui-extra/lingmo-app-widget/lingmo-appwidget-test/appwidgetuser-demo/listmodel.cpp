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

#include "listmodel.h"
#include <kappwidgetmanager.h>

#include <QDebug>

ListModel::ListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_roleName.insert(itemIconRole, "itemIcon");
    m_roleName.insert(itemTextRole, "itemName");
    appwidgetinterface = new AppWidget::KAppWidgetManager(this);
    getIconAndText();
}

QVariant ListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    // FIXME: Implement me!
    return QVariant();
}

int ListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return m_date.size();
}

QVariant ListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    switch (role) {
    case itemIconRole:
        return m_date.value(index.row()).qsItemIcon;
    case itemTextRole:
        return m_date.value(index.row()).qsItemName;
    default:
        break;
    }
    return QVariant();
}
QHash<int, QByteArray> ListModel::roleNames() const
{
    return m_roleName;
}

void ListModel::insert(QString qsItemIcon, QString qsItemName)
{
    int index = count();
    emit beginInsertRows(QModelIndex(), index, index);
    LIST_ITEM_INFO list;
    list.qsItemIcon = qsItemIcon;
    list.qsItemName = qsItemName;
    m_date.insert(index, list);
    emit endInsertRows();
}

void ListModel::remove(int index)
{
    if(index < 0 || index >= m_date.count()) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_date.removeAt(index);
    endRemoveRows();
}

int ListModel::count() const
{
    return rowCount(QModelIndex());
}

QList<LIST_ITEM_INFO> ListModel::getIconAndText()
{
    m_applist = getAppWidgetList();
    LIST_ITEM_INFO list;
    for (int var = 0; var < m_applist.count(); var++) {
        QString appwidgetname = m_applist.at(var);

        list.qsItemIcon = appwidgetinterface->getAppWidgetConfig(appwidgetname, AppWidget::PREVIEW_PATH);
        list.qsItemName = appwidgetinterface->getAppWidgetConfig(appwidgetname, AppWidget::NAME);

//用于打印查看所有小插件的属性
//        QMap<QString,QVariant> a = appwidgetinterface->getAppWidgetConfigs(appwidgetname);
//        qDebug() << "a" << a.count();
//        QMap<QString, QVariant>::const_iterator it = a.constBegin();
//        while (it != a.constEnd())
//        {
//             qDebug() << it.key() << ": " << it.value() ;
//             it++;
//        }
        m_date.append(list);
    }
    return m_date;
}

QStringList ListModel::getAppWidgetList()
{
    QStringList list = appwidgetinterface->getAppWidgetList();
    qDebug() << __FILE__ << __FUNCTION__ << ": " << list;
    return list;
}
