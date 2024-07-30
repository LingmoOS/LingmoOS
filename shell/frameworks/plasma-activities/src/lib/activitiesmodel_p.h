/*
    SPDX-FileCopyrightText: 2016 Ivan Čukić <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_ACTIVITIESMODEL_P_H
#define ACTIVITIES_ACTIVITIESMODEL_P_H

#include "activitiesmodel.h"

#include "consumer.h"

#include "utils/qflatset.h"

#include <QCollator>

namespace KActivities
{
class ActivitiesModelPrivate : public QObject
{
    Q_OBJECT
public:
    ActivitiesModelPrivate(ActivitiesModel *parent);

public Q_SLOTS:
    void onActivityNameChanged(const QString &name);
    void onActivityDescriptionChanged(const QString &description);
    void onActivityIconChanged(const QString &icon);
    void onActivityStateChanged(KActivities::Info::State state);

    void replaceActivities(const QStringList &activities);
    void onActivityAdded(const QString &id, bool notifyClients = true);
    void onActivityRemoved(const QString &id);
    void onCurrentActivityChanged(const QString &id);

    void setServiceStatus(KActivities::Consumer::ServiceStatus status);

public:
    KActivities::Consumer activities;
    QList<Info::State> shownStates;

    typedef std::shared_ptr<Info> InfoPtr;

    struct InfoPtrComparator {
        bool operator()(const InfoPtr &left, const InfoPtr &right) const
        {
            QCollator c;
            c.setCaseSensitivity(Qt::CaseInsensitive);
            c.setNumericMode(true);
            int rc = c.compare(left->name(), right->name());
            if (rc == 0) {
                return left->id() < right->id();
            }
            return rc < 0;
        }
    };

    QFlatSet<InfoPtr, InfoPtrComparator> knownActivities;
    QFlatSet<InfoPtr, InfoPtrComparator> shownActivities;

    InfoPtr registerActivity(const QString &id);
    void unregisterActivity(const QString &id);
    void showActivity(InfoPtr activityInfo, bool notifyClients);
    void hideActivity(const QString &id);
    void backgroundsUpdated(const QStringList &activities);

    InfoPtr findActivity(QObject *ptr) const;

    ActivitiesModel *const q;
};

} // namespace KActivities

#endif // ACTIVITIES_ACTIVITIESMODEL_P_H
