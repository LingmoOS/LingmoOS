/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#pragma once

#include <QDBusArgument>
#include <QDBusMetaType>

struct PortalIcon {
    QString str;
    QDBusVariant data;

    static auto registerDBusType()
    {
        return qDBusRegisterMetaType<PortalIcon>();
    }
};

QDBusArgument &operator<<(QDBusArgument &argument, const PortalIcon &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, PortalIcon &icon);

Q_DECLARE_METATYPE(PortalIcon)
