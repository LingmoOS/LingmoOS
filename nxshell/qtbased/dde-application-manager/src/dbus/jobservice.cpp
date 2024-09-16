// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dbus/jobservice.h"

JobService::JobService(const QFuture<QVariantList> &job)
    : m_job(job)
{
}

JobService::~JobService() = default;

QString JobService::status() const
{
    if (m_job.isStarted()) {
        if (m_job.isRunning()) {
            if (m_job.isSuspending()) {
                return "suspending";
            }
            if (m_job.isSuspended()) {
                return "suspended";
            }
            return "running";
        }

        if (m_job.isCanceled()) {
            return "canceled";
        }

        if (m_job.isFinished()) {
            return "finished";
        }
    }

    return "pending";
}

void JobService::Cancel()
{
    m_job.cancel();
}

void JobService::Suspend()
{
    m_job.suspend();
}

void JobService::Resume()
{
    m_job.resume();
}
