/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "job.h"

using namespace Purpose;

class Purpose::JobPrivate
{
public:
    QJsonObject m_data;
    QJsonObject m_output = {{QStringLiteral("uninitialized"), QStringLiteral("true")}};
};

Job::Job(QObject *parent)
    : KJob(parent)
    , d_ptr(new JobPrivate)
{
}

Job::~Job()
{
}

QJsonObject Job::data() const
{
    Q_D(const Job);
    return d->m_data;
}

void Job::setData(const QJsonObject &data)
{
    Q_D(Job);
    d->m_data = data;
}

QJsonObject Job::output() const
{
    Q_D(const Job);
    return d->m_output;
}

void Job::setOutput(const QJsonObject &output)
{
    Q_D(Job);
    if (d->m_output != output) {
        d->m_output = output;
        Q_EMIT outputChanged(output);
    }
}

#include "moc_job.cpp"
