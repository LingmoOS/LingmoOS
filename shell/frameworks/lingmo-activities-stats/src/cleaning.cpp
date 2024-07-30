/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QDBusReply>
#include <QDebug>

#include "cleaning.h"
#include "common/dbus/common.h"

namespace KActivities
{
namespace Stats
{
void forgetResource(Terms::Activity activities, Terms::Agent agents, const QString &resource)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KAMD_DBUS_SERVICE,
                                                          KAMD_DBUS_OBJECT_PATH("Resources/Scoring"),
                                                          KAMD_DBUS_OBJECT("ResourcesScoring"),
                                                          QStringLiteral("DeleteStatsForResource"));

    for (const auto& activity: activities.values) {
        for (const auto& agent: agents.values) {
            message.setArguments({activity, agent, resource});
            QDBusConnection::sessionBus().asyncCall(message);
        }
    }
}

void forgetResources(const Query &query)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KAMD_DBUS_SERVICE,
                                                          KAMD_DBUS_OBJECT_PATH("Resources/Scoring"),
                                                          KAMD_DBUS_OBJECT("ResourcesScoring"),
                                                          QStringLiteral("DeleteStatsForResource"));

    for (const auto& activity: query.activities()) {
        for (const auto& agent: query.agents()) {
            for (const auto& urlFilter: query.urlFilters()) {
                message.setArguments({activity, agent, urlFilter});
                QDBusConnection::sessionBus().asyncCall(message);
            }
        }
    }
}

void forgetRecentStats(Terms::Activity activities, int count, TimeUnit what)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KAMD_DBUS_SERVICE,
                                                          KAMD_DBUS_OBJECT_PATH("Resources/Scoring"),
                                                          KAMD_DBUS_OBJECT("ResourcesScoring"),
                                                          QStringLiteral("DeleteRecentStats"));

    for (const auto& activity: activities.values) {
        message.setArguments({QStringLiteral("DeleteRecentStats"),
                              activity,
                              count,
                              what == Hours      ? QStringLiteral("h")
                                  : what == Days ? QStringLiteral("d")
                                                 : QStringLiteral("m")});
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void forgetEarlierStats(Terms::Activity activities, int months)
{
    QDBusMessage message = QDBusMessage::createMethodCall(KAMD_DBUS_SERVICE,
                                                          KAMD_DBUS_OBJECT_PATH("Resources/Scoring"),
                                                          KAMD_DBUS_OBJECT("ResourcesScoring"),
                                                          QStringLiteral("DeleteEarlierStats"));

    for (const auto& activity: activities.values) {
        message.setArguments({QStringLiteral("DeleteEarlierStats"), activity, months});
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

} // namespace Stats
} // namespace KActivities
