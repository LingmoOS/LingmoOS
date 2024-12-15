// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mediamonitor.h"

#include <QDebug>

MediaMonitor::MediaMonitor(QObject *parent)
    : QObject(parent),

      m_dbusInter(new DBusInterface("org.freedesktop.DBus", "/org/freedesktop/DBus", QDBusConnection::sessionBus(), this))
{
}

void MediaMonitor::init()
{
    connect(m_dbusInter, &DBusInterface::NameOwnerChanged, this, &MediaMonitor::onNameOwnerChanged);

    for (const auto &name : m_dbusInter->ListNames().value())
        onNameOwnerChanged(name, QString(), name);
}

void MediaMonitor::onNameOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);

    if (!name.startsWith("org.mpris.MediaPlayer2."))
        return;

    if (newOwner.isEmpty())
        Q_EMIT mediaLost(name);
    else
        Q_EMIT mediaAcquired(name);
}
