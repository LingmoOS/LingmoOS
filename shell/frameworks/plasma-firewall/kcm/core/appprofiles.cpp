// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2011 Craig Drummond <craig.p.drummond@gmail.com>
// SPDX-FileCopyrightText: 2018 Alexis Lopes Zubeta <contact@azubieta.net>
// SPDX-FileCopyrightText: 2020 Tomaz Canabrava <tcanabrava@kde.org>
/*
 * UFW KControl Module
 */

#include "appprofiles.h"
#include <KConfig>
#include <KConfigGroup>
#include <QDir>

Entry::Entry(const QString &n, const QString &p)
    : name(n)
    , ports(p)
{
    ports.replace('|', ' ');
}
