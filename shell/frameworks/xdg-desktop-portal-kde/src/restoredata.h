/*
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2022 Aleix Pol <apol@kde.org>
 */

#include <QDBusArgument>
#include <QString>
#include <QVariantMap>

struct RestoreData {
    static uint currentRestoreDataVersion()
    {
        return 1;
    }

    QString session;
    quint32 version = 0;
    QVariantMap payload;
};

const QDBusArgument &operator<<(QDBusArgument &arg, const RestoreData &data);
const QDBusArgument &operator>>(const QDBusArgument &arg, RestoreData &data);
QDebug operator<<(QDebug dbg, const RestoreData &c);

Q_DECLARE_METATYPE(RestoreData)
