/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_AC_PLUGGED_JOB_P_H
#define SOLID_AC_PLUGGED_JOB_P_H

#include "job_p.h"

namespace Solid
{
class AbstractAcPluggedJob;
class AcPluggedJobPrivate : public JobPrivate
{
public:
    AcPluggedJobPrivate();

    bool plugged;
    bool backendJobFinished;
    AbstractAcPluggedJob *backendJob;
    Q_DECLARE_PUBLIC(AcPluggedJob)
};
} // Solid namespace

#endif // SOLID_AC_PLUGGED_JOB_P_H
