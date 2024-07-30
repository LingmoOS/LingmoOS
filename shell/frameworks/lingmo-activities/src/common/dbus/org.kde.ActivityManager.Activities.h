/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KAMD_ORG_KDE_ACTIVITYMANAGER_ACTIVITIES_H
#define KAMD_ORG_KDE_ACTIVITYMANAGER_ACTIVITIES_H

#include <QDBusArgument>
#include <QDebug>
#include <QList>
#include <QString>

struct ActivityInfo {
    QString id;
    QString name;
    QString description;
    QString icon;
    int state;

    ActivityInfo(const QString &id = QString(),
                 const QString &name = QString(),
                 const QString &description = QString(),
                 const QString &icon = QString(),
                 int state = 0)
        : id(id)
        , name(name)
        , description(description)
        , icon(icon)
        , state(state)
    {
    }
};

typedef QList<ActivityInfo> ActivityInfoList;

Q_DECLARE_METATYPE(ActivityInfo)
Q_DECLARE_METATYPE(ActivityInfoList)

QDBusArgument &operator<<(QDBusArgument &arg, const ActivityInfo);
const QDBusArgument &operator>>(const QDBusArgument &arg, ActivityInfo &rec);

QDebug operator<<(QDebug dbg, const ActivityInfo &r);

#endif // KAMD_ORG_KDE_ACTIVITYMANAGER_ACTIVITIES_H
