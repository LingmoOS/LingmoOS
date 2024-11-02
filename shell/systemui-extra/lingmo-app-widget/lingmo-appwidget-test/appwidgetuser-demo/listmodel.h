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

#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>

struct LIST_ITEM_INFO {
    QString qsItemIcon;
    QString qsItemName;
};

namespace AppWidget {
class KAppWidgetManager;
}
class ListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ListModel(QObject *parent = nullptr);
    enum LIST_ITEM_ROLE {
        itemIconRole = Qt::UserRole + 1,
        itemTextRole,
    };
    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Add data:
    Q_INVOKABLE void insert(QString qsItemIcon, QString qsItemName) ;

     // Remove data:
    Q_INVOKABLE void remove(int index);
    int count() const;
    QHash<int, QByteArray> roleNames() const override;

    QList<LIST_ITEM_INFO> getIconAndText();
    QStringList getAppWidgetList();

private:
    QStringList m_applist;
    QStringList m_iconlist;
    QStringList m_textlist;
    QList<LIST_ITEM_INFO> m_date;
    QHash<int, QByteArray> m_roleName;
    AppWidget::KAppWidgetManager * appwidgetinterface = nullptr;

};

#endif // LISTMODEL_H
