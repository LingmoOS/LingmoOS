/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ABSTRACT_AC_PLUGGED_JOB_H
#define ABSTRACT_AC_PLUGGED_JOB_H

#include <solid/job.h>

namespace Solid
{
class AbstractAcPluggedJob : public Job
{
    Q_OBJECT
public:
    explicit AbstractAcPluggedJob(QObject *parent = nullptr)
        : Job(parent)
    {
    }
    virtual ~AbstractAcPluggedJob()
    {
    }
    virtual bool isPlugged() const = 0;
};
}

#endif // ABSTRACT_AC_PLUGGED_JOB_H
