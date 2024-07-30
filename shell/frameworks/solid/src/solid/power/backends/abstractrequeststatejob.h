/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ABSTRACT_REQUEST_STATE_JOB_H
#define ABSTRACT_REQUEST_STATE_JOB_H

#include "power.h"
#include <solid/job.h>

namespace Solid
{
class AbstractRequestStateJob : public Job
{
    Q_OBJECT
public:
    explicit AbstractRequestStateJob(QObject *parent = nullptr)
        : Job(parent)
    {
    }
    virtual ~AbstractRequestStateJob()
    {
    }

    Power::InhibitionType state;
};
}

#endif // ABSTRACT_REQUEST_STATE_JOB_H
