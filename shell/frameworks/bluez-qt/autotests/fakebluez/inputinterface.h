/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef INPUTINTERFACE_H
#define INPUTINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>

class QDBusObjectPath;

class InputObject : public QObject
{
public:
    explicit InputObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class InputInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.Input1")
    Q_PROPERTY(QString ReconnectMode READ reconnectMode)

public:
    explicit InputInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    QString reconnectMode() const;
};

#endif // INPUTINTERFACE_H
