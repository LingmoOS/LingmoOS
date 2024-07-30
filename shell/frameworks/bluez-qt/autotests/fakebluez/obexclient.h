/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef OBEXCLIENT_H
#define OBEXCLIENT_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusMessage;

class ObexClient : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.obex.Client1")

public:
    explicit ObexClient(QObject *parent = nullptr);

    void runAction(const QString &actionName, const QVariantMap &properties);

public Q_SLOTS:
    void CreateSession(const QString &destination, const QVariantMap &args, const QDBusMessage &msg);
    void RemoveSession(const QDBusObjectPath &session, const QDBusMessage &msg);
};

#endif // OBEXCLIENT_H
