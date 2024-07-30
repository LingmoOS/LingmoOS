/*
 * SPDX-FileCopyrightText: 2016 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>
 */

#include "portalicon.h"

QDBusArgument &operator<<(QDBusArgument &argument, const PortalIcon &icon)
{
    argument.beginStructure();
    argument << icon.str << icon.data;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PortalIcon &icon)
{
    argument.beginStructure();
    argument >> icon.str >> icon.data;
    argument.endStructure();
    return argument;
}
