/*
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "global.h"

#include "backends/abstractcontact.h"
#include "personmanager_p.h"
#include "personpluginmanager.h"

// these namespace members expose the useful bits of PersonManager
// global.h should be included from every exported header file so namespace members are always visible

QString KPeople::mergeContacts(const QStringList &uris)
{
    return PersonManager::instance()->mergeContacts(uris);
}

bool KPeople::unmergeContact(const QString &uri)
{
    return PersonManager::instance()->unmergeContact(uri);
}

QString KPeople::iconNameForPresenceString(const QString &presenceName)
{
    if (presenceName == QLatin1String("available")) {
        return QStringLiteral("user-online");
    }

    if (presenceName == QLatin1String("away")) {
        return QStringLiteral("user-away");
    }

    if (presenceName == QLatin1String("busy") || presenceName == QLatin1String("dnd")) {
        return QStringLiteral("user-busy");
    }

    if (presenceName == QLatin1String("xa")) {
        return QStringLiteral("user-away-extended");
    }

    if (presenceName == QLatin1String("hidden")) {
        return QStringLiteral("user-invisible");
    }

    return QStringLiteral("user-offline");
}

int KPeople::presenceSortPriority(const QString &presenceName)
{
    if (presenceName == QLatin1String("available")) {
        return 0;
    }

    if (presenceName == QLatin1String("busy") || presenceName == QLatin1String("dnd")) {
        return 1;
    }

    if (presenceName == QLatin1String("hidden")) {
        return 2;
    }

    if (presenceName == QLatin1String("away")) {
        return 3;
    }

    if (presenceName == QLatin1String("xa")) {
        return 4;
    }

    if (presenceName == QLatin1String("unknown")) {
        return 5;
    }

    if (presenceName == QLatin1String("offline")) {
        return 6;
    }

    return 7;
}

void initResources()
{
    Q_INIT_RESOURCE(resources);
}
