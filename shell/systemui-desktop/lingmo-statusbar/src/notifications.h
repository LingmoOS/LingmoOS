/*
 * Copyright (C) 2024 - 2022 LingmoOS Team.
 *
 * Author:     Reion Wong <reion@lingmo.org>
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

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCall>

class Notifications : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool doNotDisturb READ doNotDisturb WRITE setDoNotDisturb NOTIFY doNotDisturbChanged)

public:
    explicit Notifications(QObject *parent = nullptr);

    bool doNotDisturb() const;
    void setDoNotDisturb(bool enabled);

private slots:
    void onDBusDoNotDisturbChanged();

signals:
    void doNotDisturbChanged();

private:
    QDBusInterface m_iface;
    bool m_doNotDisturb;
};

#endif // NOTIFICATIONS_H
