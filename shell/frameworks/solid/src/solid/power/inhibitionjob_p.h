/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_INHIBITION_JOB_PRIVATE_H
#define SOLID_INHIBITION_JOB_PRIVATE_H

#include "job_p.h"
#include "power.h"

namespace Solid
{
class Inhibition;
class AbstractInhibitionJob;
class InhibitionJobPrivate : public JobPrivate
{
public:
    InhibitionJobPrivate();

    QString description;
    Power::InhibitionTypes inhibitions;
    Inhibition *inhibition;
    AbstractInhibitionJob *backendJob;
    Q_DECLARE_PUBLIC(InhibitionJob)
};
} // Solid namespace

#endif // SOLID_ADD_INHIBITION_JOB_PRIVATE_H
