/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_JOB_PRIVATE
#define SOLID_JOB_PRIVATE

#include "job.h"

class QEventLoop;

namespace Solid
{
class JobPrivate
{
public:
    JobPrivate();
    virtual ~JobPrivate() = default;

    Job *q_ptr;
    int error;
    QString errorText;
    QEventLoop *eventLoop;
    Q_DECLARE_PUBLIC(Job)
};
}
#endif // SOLID_JOB_PRIVATE
