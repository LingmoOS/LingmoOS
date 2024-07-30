/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_MANAGER_P
#define ACTIVITIES_MANAGER_P

#include <common/dbus/org.kde.ActivityManager.Activities.h>

#include "activities_interface.h"
#include "application_interface.h"
#include "features_interface.h"
#include "resources_interface.h"
#include "resources_linking_interface.h"

#include <QDBusServiceWatcher>

namespace Service = org::kde::ActivityManager;

namespace KActivities
{
class Manager : public QObject
{
    Q_OBJECT

public:
    static Manager *self();

    static bool isServiceRunning();

    static Service::Activities *activities();
    static Service::Resources *resources();
    static Service::ResourcesLinking *resourcesLinking();
    static Service::Features *features();

public Q_SLOTS:
    void serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

Q_SIGNALS:
    void serviceStatusChanged(bool status);

private:
    Manager();

    QDBusServiceWatcher m_watcher;

    static Manager *s_instance;

    Service::Application *const m_service;
    Service::Activities *const m_activities;
    Service::Resources *const m_resources;
    Service::ResourcesLinking *const m_resourcesLinking;
    Service::Features *const m_features;
    bool m_serviceRunning;

    friend class ManagerInstantiator;
};

} // namespace KActivities

#endif // ACTIVITIES_MANAGER_P
