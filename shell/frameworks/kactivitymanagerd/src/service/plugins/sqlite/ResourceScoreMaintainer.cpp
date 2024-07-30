/*
 *   SPDX-FileCopyrightText: 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

// Self
#include "ResourceScoreMaintainer.h"

// Qt
#include <QList>

// System
#include <unistd.h>

// Utils
#include <utils/d_ptr_implementation.h>
#include <utils/for_each_assoc.h>

// Local
#include "ResourceScoreCache.h"
#include "StatsPlugin.h"

class ResourceScoreMaintainer::Private
{
public:
    Private()
    {
    }

    ~Private();

    typedef QString ApplicationName;
    typedef QString ActivityID;
    typedef QList<QString> ResourceList;

    typedef QHash<ApplicationName, ResourceList> Applications;
    typedef QHash<ActivityID, Applications> ResourceTree;

    ResourceTree scheduledResources;

    void processActivity(const ActivityID &activity, const Applications &applications);

    void processResources();

    QTimer processResourcesTimer;
};

ResourceScoreMaintainer::Private::~Private()
{
}

void ResourceScoreMaintainer::Private::processResources()
{
    using namespace kamd::utils;

    // initial delay before processing the resources
    sleep(1);

    ResourceTree resources;

    std::swap(resources, scheduledResources);

    const auto activity = StatsPlugin::self()->currentActivity();

    // Let us first process the events related to the current
    // activity so that the stats are available quicker

    if (resources.contains(activity)) {
        processActivity(activity, resources[activity]);
        resources.remove(activity);
    }

    for_each_assoc(resources, [this](const ActivityID &activity, const Applications &applications) {
        processActivity(activity, applications);
    });
}

void ResourceScoreMaintainer::Private::processActivity(const ActivityID &activity, const Applications &applications)
{
    using namespace kamd::utils;

    for_each_assoc(applications, [&](const ApplicationName &application, const ResourceList &resources) {
        for (const auto &resource : resources) {
            ResourceScoreCache(activity, application, resource).update();
        }
    });
}

ResourceScoreMaintainer *ResourceScoreMaintainer::self()
{
    static ResourceScoreMaintainer instance;
    return &instance;
}

ResourceScoreMaintainer::ResourceScoreMaintainer()
{
    d->processResourcesTimer.setInterval(1000);
    d->processResourcesTimer.setSingleShot(true);
    connect(&d->processResourcesTimer, &QTimer::timeout, this, [this] {
        d->processResources();
    });
}

ResourceScoreMaintainer::~ResourceScoreMaintainer()
{
}

void ResourceScoreMaintainer::processResource(const QString &resource, const QString &application)
{
    // Checking whether the item is already scheduled for
    // processing

    const auto activity = StatsPlugin::self()->currentActivity();

    Q_ASSERT_X(!application.isEmpty(), "ResourceScoreMaintainer::processResource", "Agent should not be empty");
    Q_ASSERT_X(!resource.isEmpty(), "ResourceScoreMaintainer::processResource", "Resource should not be empty");

    if (d->scheduledResources.contains(activity) && d->scheduledResources[activity].contains(application)
        && d->scheduledResources[activity][application].contains(resource)) {
        // Nothing

    } else {
        d->scheduledResources[activity][application] << resource;
    }

    d->processResourcesTimer.start();
}
