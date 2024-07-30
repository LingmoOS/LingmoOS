/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "profilemanager.h"

#include <QDBusMessage>

ProfileManager::ProfileManager(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.ProfileManager1"));
    setPath(QDBusObjectPath(QStringLiteral("/org/bluez")));
}

void ProfileManager::RegisterProfile(const QDBusObjectPath &path, const QString &uuid, const QVariantMap &options, const QDBusMessage &msg)
{
    Q_UNUSED(path)
    Q_UNUSED(uuid)
    Q_UNUSED(options)
    Q_UNUSED(msg)
}

void ProfileManager::UnregisterProfile(const QDBusObjectPath &path, const QDBusMessage &msg)
{
    Q_UNUSED(path)
    Q_UNUSED(msg)
}

#include "moc_profilemanager.cpp"
