/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "statesjob.h"
#include "backends/abstractstatesjob.h"
#include "powerbackendloader.h"
#include "statesjob_p.h"

#include <QDebug>

using namespace Solid;

StatesJobPrivate::StatesJobPrivate()
{
    backendJob = nullptr;
}

StatesJob::StatesJob(QObject *parent)
    : Job(*new StatesJobPrivate(), parent)
{
}

void StatesJob::doStart()
{
    Q_D(StatesJob);
    d->backendJob = PowerBackendLoader::statesJob();
    connect(d->backendJob, &AbstractStatesJob::result, [this]() {
        emitResult();
    });

    d->backendJob->start();
}

Power::InhibitionTypes StatesJob::states() const
{
    if (d_func()->backendJob) {
        return d_func()->backendJob->states();
    }
    qWarning() << "statesJob called without having called start";
    return Power::None;
}

#include "moc_statesjob.cpp"
