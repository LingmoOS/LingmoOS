/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2024 Reion Wong <aj@lingmo.org>                     *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
 *   Copyright (C) 2009 Matthieu Gallien <matthieu_gallien@yahoo.fr>       *
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

#ifndef STATUSNOTIFIERWATCHER_H
#define STATUSNOTIFIERWATCHER_H

#include <QObject>
#include <QDBusContext>
#include <QStringList>
#include <QSet>

class QDBusServiceWatcher;
class StatusNotifierWatcher : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_SCRIPTABLE Q_PROPERTY(bool IsStatusNotifierHostRegistered READ IsStatusNotifierHostRegistered)
    Q_SCRIPTABLE Q_PROPERTY(int ProtocolVersion READ protocolVersion)
    Q_SCRIPTABLE Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ RegisteredStatusNotifierItems)

public:
    explicit StatusNotifierWatcher(QObject *parent = nullptr);
    ~StatusNotifierWatcher();

    QStringList RegisteredStatusNotifierItems() const;
    bool IsStatusNotifierHostRegistered() const;
    int protocolVersion() const { return 0; }

public slots:
    void RegisterStatusNotifierItem(const QString &service);
    void RegisterStatusNotifierHost(const QString &service);

protected Q_SLOTS:
    void serviceUnregistered(const QString &name);

Q_SIGNALS:
    void StatusNotifierItemRegistered(const QString &service);
    // TODO: decide if this makes sense, the systray itself could notice the vanishing of items, but looks complete putting it here
    void StatusNotifierItemUnregistered(const QString &service);
    void StatusNotifierHostRegistered();
    void StatusNotifierHostUnregistered();

private:
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    QStringList m_registeredServices;
    QSet<QString> m_statusNotifierHostServices;
};

#endif // STATUSNOTIFIERWATCHER_H
