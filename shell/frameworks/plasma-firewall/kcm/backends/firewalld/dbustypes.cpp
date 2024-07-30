// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */
#include "dbustypes.h"

const QDBusArgument &operator>>(const QDBusArgument &argument, firewalld_reply &mystruct)
{
    argument.beginStructure();
    argument >> mystruct.ipv >> mystruct.table >> mystruct.chain >> mystruct.priority >> mystruct.rules;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator<<(QDBusArgument &argument, const firewalld_reply &mystruct)
{
    argument.beginStructure();
    argument << mystruct.ipv << mystruct.table << mystruct.chain << mystruct.priority << mystruct.rules;
    argument.endStructure();
    return argument;
}
