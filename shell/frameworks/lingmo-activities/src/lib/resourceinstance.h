/*
    SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_RESOURCEINSTANCE_H
#define ACTIVITIES_RESOURCEINSTANCE_H

#include <QObject>
#include <QUrl>

#include "lingmo_activities_export.h"

namespace KActivities::ResourceInstance
{
/**
 * This function is used to notify the system that a file, web page
 * or some other resource has been accessed.
 *
 * @param uri URI of the resource
 * @param application application's name (the name used for the .desktop file).
 *        If not specified, QCoreApplication::applicationName is used
 */
LINGMO_ACTIVITIES_EXPORT void notifyAccessed(const QUrl &uri, const QString &application = QString());
}

#endif // ACTIVITIES_RESOURCEINSTANCE_H
