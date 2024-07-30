/*
    SPDX-FileCopyrightText: 2010 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "activityengine.h"

#include <Lingmo5Support/Service>
#include <Lingmo5Support/ServiceJob>

using namespace Lingmo5Support;

namespace KActivities
{
class Controller;
} // namespace KActivities

class ActivityService : public Lingmo5Support::Service
{
    Q_OBJECT

public:
    ActivityService(KActivities::Controller *controller, const QString &source);
    ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;

private:
    KActivities::Controller *m_activityController;
    QString m_id;
};
