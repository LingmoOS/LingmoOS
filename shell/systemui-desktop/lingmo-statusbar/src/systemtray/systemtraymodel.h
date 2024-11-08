/***************************************************************************
 *   Copyright (C) 2024 Reion Wong     <aj@lingmo.org>                 *
 *   Copyright (C) 2020 Konrad Materka <materka@gmail.com>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef SYSTEMTRAYMODEL_H
#define SYSTEMTRAYMODEL_H

#include <QAbstractListModel>

#include <QQuickItem>
#include <QQuickWindow>

#include "statusnotifierwatcher.h"
#include "statusnotifieritemhost.h"
#include "statusnotifieritemsource.h"

class SystemTrayModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        IconNameRole,
        IconRole,
        TitleRole,
        ToolTipRole,
        CanColorOverlay
    };

    explicit SystemTrayModel(QObject *parent = nullptr);
    ~SystemTrayModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int indexOf(const QString &id);
    StatusNotifierItemSource *findItemById(const QString &id);

    Q_INVOKABLE void leftButtonClick(const QString &id, int x, int y);
    Q_INVOKABLE void rightButtonClick(const QString &id, QQuickItem *iconItem, int x, int y);
    Q_INVOKABLE void middleButtonClick(const QString &id, int x, int y);

    Q_INVOKABLE void move(int from, int to);

    /**
     * Find out global coordinates for a popup given local MouseArea
     * coordinates
     * from plasma-workspace/applets/systemtray/systemtray.cpp
     */
    Q_INVOKABLE QPointF popupPosition(QQuickItem *visualParent, int x, int y);

private slots:
    void onItemAdded(const QString &service);
    void onItemRemoved(const QString &service);
    void updated(StatusNotifierItemSource *item);

private:
    StatusNotifierWatcher *m_watcher;
    StatusNotifierItemHost *m_sniHost;
    QList<StatusNotifierItemSource *> m_items;
    QString m_hostName;
};

#endif // SYSTEMTRAYMODEL_H
