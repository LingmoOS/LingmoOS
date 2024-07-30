/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ABSTRACT_STATES_JOB_H
#define ABSTRACT_STATES_JOB_H

#include "power.h"
#include <solid/job.h>

namespace Solid
{
class AbstractStatesJob : public Job
{
    Q_OBJECT
public:
    explicit AbstractStatesJob(QObject *parent = nullptr)
        : Job(parent)
    {
    }
    virtual ~AbstractStatesJob()
    {
    }

    virtual Power::InhibitionTypes states() const = 0;
};
}

#endif // ABSTRACT_STATES_JOB_H
