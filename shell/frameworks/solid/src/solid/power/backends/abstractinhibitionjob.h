/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ABSTRACT_INHIBITION_JOB_H
#define ABSTRACT_INHIBITION_JOB_H

#include <solid/job.h>

namespace Solid
{
class Inhibition;
class AbstractInhibitionJob : public Job
{
    Q_OBJECT
public:
    explicit AbstractInhibitionJob(QObject *parent = nullptr)
        : Job(parent)
    {
    }
    virtual ~AbstractInhibitionJob()
    {
    }

    virtual Inhibition *inhibition() = 0;
};
}

#endif // ABSTRACT_ADD_INHIBITION_JOB_H
