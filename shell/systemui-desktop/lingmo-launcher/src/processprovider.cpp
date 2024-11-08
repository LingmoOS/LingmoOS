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

#include "processprovider.h"
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QProcess>

ProcessProvider::ProcessProvider(QObject *parent)
    : QObject(parent)
{

}

bool ProcessProvider::startDetached(const QString &exec, QStringList args)
{
    QDBusInterface iface("com.lingmo.Session",
                         "/Session",
                         "com.lingmo.Session", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.asyncCall("launch", exec, args).waitForFinished();
        return true;
    } else {
        QProcess process;
        process.startDetached(exec, args);
    }

    return false;
}
