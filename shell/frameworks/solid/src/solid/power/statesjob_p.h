/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_SLEEP_STATES_JOB_P_H
#define SOLID_SLEEP_STATES_JOB_P_H

#include "job_p.h"

namespace Solid
{
class AbstractStatesJob;
class StatesJobPrivate : public JobPrivate
{
public:
    StatesJobPrivate();

    bool plugged;
    AbstractStatesJob *backendJob;
    Q_DECLARE_PUBLIC(StatesJob)
};
} // Solid namespace

#endif // SOLID_SLEEP_STATES_JOB_P_H
