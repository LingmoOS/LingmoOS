/*
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "jobcontroller.h"

#include "configuration.h"

namespace Purpose
{
void JobController::configure()
{
    Q_ASSERT(m_model);
    Q_ASSERT(m_index >= 0);

    m_configuration = m_model->configureJob(m_index);
    m_configuration->setUseSeparateProcess(false);
    Q_EMIT configChanged();

    if (m_configuration->isReady()) {
        startJob();
    } else {
        m_state = Configuring;
        Q_EMIT stateChanged();
    }
}

void JobController::startJob()
{
    m_job = m_configuration->createJob();
    Q_ASSERT(m_job);
    Q_EMIT jobChanged();

    connect(m_job, &KJob::result, this, [this](KJob *job) {
        if (job->error()) {
            m_state = Error;
        } else {
            m_state = Finished;
        }

        Q_EMIT stateChanged();
    });

    m_job->start();

    m_state = Running;
    Q_EMIT stateChanged();
}

void JobController::cancel()
{
    m_state = Cancelled;
    Q_EMIT stateChanged();
}

AlternativesModel *JobController::model() const
{
    return m_model;
}

void JobController::setModel(AlternativesModel *model)
{
    if (m_model != model) {
        m_model = model;
        Q_EMIT modelChanged();
    }
}

int JobController::index() const
{
    return m_index;
}

void JobController::setIndex(int index)
{
    if (index != m_index) {
        m_index = index;
        Q_EMIT indexChanged();
    }
}

JobController::State JobController::state() const
{
    return m_state;
}

Configuration *JobController::config() const
{
    return m_configuration;
}

Job *JobController::job() const
{
    return m_job;
}

}

#include "moc_jobcontroller.cpp"
