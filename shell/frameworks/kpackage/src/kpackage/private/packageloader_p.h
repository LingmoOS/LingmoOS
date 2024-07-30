/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGELOADER_P_H
#define KPACKAGE_PACKAGELOADER_P_H

#include "packagestructure.h"
#include <KPluginMetaData>
#include <QHash>
#include <QPointer>

namespace KPackage
{
class PackageLoaderPrivate
{
public:
    QHash<QString, QPointer<PackageStructure>> structures;
    // We only use this cache during start of the process to speed up many consecutive calls
    // After that, we're too afraid to produce race conditions and it's not that time-critical anyway
    // the 20 seconds here means that the cache is only used within 20sec during startup, after that,
    // complexity goes up and we'd have to update the cache in order to avoid subtle bugs
    // just not using the cache is way easier then, since it doesn't make *that* much of a difference,
    // anyway
    int maxCacheAge = 20;
    qint64 pluginCacheAge = 0;
    QHash<QString, QList<KPluginMetaData>> pluginCache;
};

}

#endif
