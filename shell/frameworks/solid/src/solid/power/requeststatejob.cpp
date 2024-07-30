/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "requeststatejob.h"
#include "backends/abstractrequeststatejob.h"
#include "powerbackendloader.h"
#include "requeststatejob_p.h"

using namespace Solid;

RequestStateJobPrivate::RequestStateJobPrivate()
{
    backendJob = nullptr;
    state = Power::None;
}

RequestStateJob::RequestStateJob(QObject *parent)
    : Job(*new RequestStateJobPrivate(), parent)
{
}

void RequestStateJob::setState(Power::InhibitionType state)
{
    Q_D(RequestStateJob);
    d->state = state;
}

void RequestStateJob::doStart()
{
    Q_D(RequestStateJob);
    d->backendJob = PowerBackendLoader::requestState();
    d->backendJob->state = d->state;

    connect(d->backendJob, &AbstractRequestStateJob::result, [this](Job *job) {
        if (job->error()) {
            setError(job->error());
            setErrorText(job->errorText());
        }
        emitResult();
    });

    d->backendJob->start();
}

#include "moc_requeststatejob.cpp"
