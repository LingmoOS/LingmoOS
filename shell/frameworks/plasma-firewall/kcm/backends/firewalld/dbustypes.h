// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
/*
 * Firewalld backend for plasma firewall
 */

#ifndef FIREWALLDCLIENTDBUS_H
#define FIREWALLDCLIENTDBUS_H

#include <QDBusArgument>

struct firewalld_reply {
    QString ipv;
    QString table;
    QString chain;
    int priority = 0;
    QStringList rules;
};

Q_DECLARE_METATYPE(firewalld_reply);
const QDBusArgument &operator>>(const QDBusArgument &argument, firewalld_reply &mystruct);
const QDBusArgument &operator<<(QDBusArgument &argument, const firewalld_reply &mystruct);

#endif
