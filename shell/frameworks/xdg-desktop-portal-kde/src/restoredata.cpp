/*
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2022 Aleix Pol <apol@kde.org>
 */

#include "restoredata.h"

#include <QDBusMetaType>
#include <QDataStream>
#include <QIODevice>

const QDBusArgument &operator<<(QDBusArgument &arg, const RestoreData &data)
{
    arg.beginStructure();
    arg << data.session;
    arg << data.version;

    QByteArray payloadSerialised;
    {
        QDataStream ds(&payloadSerialised, QIODevice::WriteOnly);
        ds << data.payload;
    }

    arg << QDBusVariant(payloadSerialised);
    arg.endStructure();
    return arg;
}
const QDBusArgument &operator>>(const QDBusArgument &arg, RestoreData &data)
{
    arg.beginStructure();
    arg >> data.session;
    arg >> data.version;

    QDBusVariant payloadVariant;
    arg >> payloadVariant;
    {
        QByteArray payloadSerialised = payloadVariant.variant().toByteArray();
        QDataStream ds(&payloadSerialised, QIODevice::ReadOnly);
        ds >> data.payload;
    }
    arg.endStructure();
    return arg;
}

QDebug operator<<(QDebug dbg, const RestoreData &c)
{
    dbg.nospace() << "RestoreData(" << c.session << ", " << c.version << ", " << c.payload << ")";
    return dbg.space();
}
