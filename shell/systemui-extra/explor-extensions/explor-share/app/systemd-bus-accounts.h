/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef SYSTEMDBUSACCOUNTS_H
#define SYSTEMDBUSACCOUNTS_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusReply>

class SystemDbusAccounts : public QObject
{
    Q_OBJECT
public:
    explicit SystemDbusAccounts(QObject *parent = nullptr);
    ~SystemDbusAccounts();

    QStringList getListCachedUsers();
    QStringList getAllUserNames();
    QString getUserName(QString objectPath);

public Q_SLOTS:
    void createUserSuccess(QDBusObjectPath objectPath);
    void deleteUserSuccess(QDBusObjectPath objectPath);

Q_SIGNALS:
    void createUserDone(QString path);
    void deleteUserDone(QString path);

private:
    QDBusInterface *m_systemInterface = nullptr;

};

#endif // SYSTEMDBUSACCOUNTS_H
