/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
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
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#include "sync-thread.h"
#include <QThread>
#include <QDebug>
#include <QDBusInterface>
#include <libnotify/notify.h>

using namespace Peony;

SyncThread::SyncThread(QString uri, QObject *parent) : QObject(parent), mUri(uri)
{
    mHint = QObject::tr("It need to synchronize before operating the device,place wait!");
}

void SyncThread::parentStartedSlot()
{
    if (mUri == "file:///"
            || mUri == "computer:///root.link") {
        return;
    }

    QProcess syncProcess;

    notifyUser(mHint);

    syncProcess.start("/usr/bin/sync");
    syncProcess.waitForFinished(-1);

    Q_EMIT this->syncFinished();
}

void SyncThread::notifyUser(QString notifyContent)
{
    QDBusInterface iface ("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications", QDBusConnection::sessionBus ());

    QList <QVariant> args;
    QStringList actions;
    QMap <QString, QVariant> hints;

    args << QObject::tr("File Manager").toUtf8().constData()
         << ((unsigned int) 0)
         << "system-file-manager"
         << tr("notify")
         << notifyContent
         << actions
         << hints
         << (int) -1;

    iface.callWithArgumentList (QDBus::AutoDetect, "Notify", args);
}

