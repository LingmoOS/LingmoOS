/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef BLUEZQT_JOB_PRIVATE
#define BLUEZQT_JOB_PRIVATE

#include "job.h"

class QEventLoop;

namespace BluezQt
{
class JobPrivate
{
public:
    JobPrivate();

    Job *q_ptr;
    int error;
    QString errorText;
    bool running;
    bool finished;
    bool killed;
    QEventLoop *eventLoop;

    Q_DECLARE_PUBLIC(Job)
};

} // namespace BluezQt

#endif // BLUEZQT_JOB_PRIVATE
