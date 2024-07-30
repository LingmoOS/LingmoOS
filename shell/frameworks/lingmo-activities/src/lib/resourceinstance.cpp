/*
    SPDX-FileCopyrightText: 2011-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "resourceinstance.h"
#include "manager_p.h"

#include <QCoreApplication>

namespace KActivities::ResourceInstance
{

void notifyAccessed(const QUrl &uri, const QString &application)
{
    if (uri.isEmpty()) {
        return;
    }

    if (uri.fileName().startsWith('.')) {
        // skip hidden files
        return;
    }

    if (uri.path().contains(QStringLiteral("/."))) {
        // skip files in hidden directories
        return;
    }

    Manager::resources()->RegisterResourceEvent(application.isEmpty() ? QCoreApplication::instance()->applicationName() : application,
                                                0,
                                                uri.toString(),
                                                0 /* Accessed */);
}

} // namespace KActivities::ResourceInstance
