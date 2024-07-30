/*
 *   SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "org.kde.ActivityManager.Activities.h"

#include <QDBusMetaType>

namespace details
{
class ActivityInfoStaticInit
{
public:
    ActivityInfoStaticInit()
    {
        qDBusRegisterMetaType<ActivityInfo>();
        qDBusRegisterMetaType<ActivityInfoList>();
    }

    static ActivityInfoStaticInit _instance;
};

ActivityInfoStaticInit ActivityInfoStaticInit::_instance;

} // namespace details

QDBusArgument &operator<<(QDBusArgument &arg, const ActivityInfo &r)
{
    arg.beginStructure();

    arg << r.id;
    arg << r.name;
    arg << r.description;
    arg << r.icon;
    arg << r.state;

    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ActivityInfo &r)
{
    arg.beginStructure();

    arg >> r.id;
    arg >> r.name;
    arg >> r.description;
    arg >> r.icon;
    arg >> r.state;

    arg.endStructure();

    return arg;
}

QDebug operator<<(QDebug dbg, const ActivityInfo &r)
{
    dbg << "ActivityInfo(" << r.id << r.name << ")";
    return dbg.space();
}
