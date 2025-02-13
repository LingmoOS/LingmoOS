/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Lingmo OS Team <reionwong@gmail.com>
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

#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>
#include <QDBusInterface>

class PowerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(int idleTime READ idleTime WRITE setIdleTime NOTIFY idleTimeChanged)
    Q_PROPERTY(int hibernateTime READ hibernateTime WRITE setHibernateTime NOTIFY hibernateTimeChanged)
    Q_PROPERTY(bool sleepWhenClosedScreen READ sleepWhenClosedScreen WRITE setSleepWhenClosedScreen NOTIFY sleepWhenClosedScreenChanged)
    Q_PROPERTY(bool lockWhenClosedScreen READ lockWhenClosedScreen WRITE setLockWhenClosedScreen NOTIFY lockWhenClosedScreenChanged)

public:
    explicit PowerManager(QObject *parent = nullptr);

    int mode() const;
    void setMode(int mode);

    int idleTime();
    void setIdleTime(int idleTime);

    int hibernateTime();
    void setHibernateTime(int timeout);

    bool sleepWhenClosedScreen() const;
    void setSleepWhenClosedScreen(bool sleepWhenClosedScreen);

    bool lockWhenClosedScreen() const;
    void setLockWhenClosedScreen(bool lockWhenClosedScreen);

signals:
    void modeChanged();
    void idleTimeChanged();
    void hibernateTimeChanged();
    void sleepWhenClosedScreenChanged();
    void lockWhenClosedScreenChanged();

private:
    QDBusInterface m_iface;
    int m_mode;
    int m_idleTime;
    int m_hibernateTime;
    bool m_sleepWhenClosedScreen;
    bool m_lockWhenClosedScreen;
};

#endif // POWERMANAGER_H
