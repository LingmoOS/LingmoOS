/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class ProfileManager : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.ProfileManager1")

public:
    explicit ProfileManager(QObject *parent = nullptr);

public Q_SLOTS:
    void RegisterProfile(const QDBusObjectPath &path, const QString &uuid, const QVariantMap &options, const QDBusMessage &msg);
    void UnregisterProfile(const QDBusObjectPath &path, const QDBusMessage &msg);
};

#endif // PROFILEMANAGER_H
