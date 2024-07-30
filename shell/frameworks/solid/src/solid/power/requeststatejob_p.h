/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_REQUEST_STATE_JOB_PRIVATE_H
#define SOLID_REQUEST_STATE_JOB_PRIVATE_H

#include "job_p.h"
#include "power.h"

namespace Solid
{
class AbstractRequestStateJob;
class RequestStateJobPrivate : public JobPrivate
{
public:
    RequestStateJobPrivate();

    Power::InhibitionType state;

    AbstractRequestStateJob *backendJob;
    Q_DECLARE_PUBLIC(RequestStateJob)
};
} // Solid namespace

#endif // SOLID_REQUEST_STATE_JOB_PRIVATE_H
