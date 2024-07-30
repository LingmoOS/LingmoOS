/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2013 Daniel Schaal <farbing@web.de>
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_DBUSTYPES_H
#define BLUEZQT_DBUSTYPES_H

#include <QDBusObjectPath>
#include <QVariantMap>

typedef QList<QVariantMap> QVariantMapList;
Q_DECLARE_METATYPE(QVariantMapList)

typedef QMap<QString, QVariantMap> QVariantMapMap;
Q_DECLARE_METATYPE(QVariantMapMap)

typedef QMap<QDBusObjectPath, QVariantMapMap> DBusManagerStruct;
Q_DECLARE_METATYPE(DBusManagerStruct)

#endif // BLUEZQT_DBUSTYPES_H
