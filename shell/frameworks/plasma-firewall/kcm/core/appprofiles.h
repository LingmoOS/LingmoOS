// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>

#ifndef UFW_APP_PROFILES_H
#define UFW_APP_PROFILES_H
/*
 * UFW KControl Module
 */

#include <kcm_firewall_core_export.h>

#include <QList>
#include <QString>

struct KCM_FIREWALL_CORE_EXPORT Entry {
    Entry(const QString &n, const QString &p = QString());
    bool operator<(const Entry &o) const
    {
        return name.localeAwareCompare(o.name) < 0;
    }
    bool operator==(const Entry &o) const
    {
        return name == o.name;
    }
    QString name;
    QString ports;
    //     Types::Protocol protocol;
};

#endif
