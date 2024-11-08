/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     rekols <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef APPLICATIONMODEL_H
#define APPLICATIONMODEL_H

#include <QAbstractListModel>
#include "applicationitem.h"
#include "systemappmonitor.h"
#include "xwindowinterface.h"

class ApplicationModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        AppIdRole = Qt::UserRole + 1,
        IconNameRole,
        VisibleNameRole,
        ActiveRole,
        WindowCountRole,
        IsPinnedRole,
        DesktopFileRole,
        FixedItemRole
    };

    explicit ApplicationModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addItem(const QString &desktopFile);
    void removeItem(const QString &desktopFile);
    bool desktopContains(const QString &desktopFile);
    bool isDesktopPinned(const QString &desktopFile);

    Q_INVOKABLE void save() { savePinAndUnPinList(); }

    Q_INVOKABLE void clicked(const QString &id);
    Q_INVOKABLE void raiseWindow(const QString &id);

    Q_INVOKABLE bool openNewInstance(const QString &appId);
    Q_INVOKABLE void closeAllByAppId(const QString &appId);
    Q_INVOKABLE void pin(const QString &appId);
    Q_INVOKABLE void unPin(const QString &appId);

    Q_INVOKABLE void updateGeometries(const QString &id, QRect rect);

    Q_INVOKABLE void move(int from, int to);

signals:
    void countChanged();

    void itemAdded();
    void itemRemoved();

private:
    ApplicationItem *findItemByWId(quint64 wid);
    ApplicationItem *findItemById(const QString &id);
    ApplicationItem *findItemByDesktop(const QString &desktop);

    bool contains(const QString &id);
    int indexOf(const QString &id);
    void initPinnedApplications();
    void savePinAndUnPinList();

    void handleDataChangedFromItem(ApplicationItem *item);

    void onWindowAdded(quint64 wid);
    void onWindowRemoved(quint64 wid);
    void onActiveChanged(quint64 wid);

private:
    XWindowInterface *m_iface;
    SystemAppMonitor *m_sysAppMonitor;
    QList<ApplicationItem *> m_appItems;
};

#endif // APPLICATIONMODEL_H
