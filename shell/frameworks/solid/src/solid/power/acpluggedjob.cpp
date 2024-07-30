/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "acpluggedjob.h"
#include "acpluggedjob_p.h"
#include "backends/abstractacpluggedjob.h"
#include "powerbackendloader.h"

#include <QDebug>

using namespace Solid;

AcPluggedJobPrivate::AcPluggedJobPrivate()
{
    backendJob = nullptr;
    plugged = false;
    backendJobFinished = false;
}

AcPluggedJob::AcPluggedJob(QObject *parent)
    : Job(*new AcPluggedJobPrivate(), parent)
{
}

void AcPluggedJob::doStart()
{
    Q_D(AcPluggedJob);
    d->backendJob = PowerBackendLoader::AcPluggedJob();
    connect(d->backendJob, &AbstractAcPluggedJob::result, [this, d]() {
        d->backendJobFinished = true;
        d->plugged = d->backendJob->isPlugged();
        emitResult();
    });

    d->backendJob->start();
}

bool AcPluggedJob::isPlugged() const
{
    if (d_func()->backendJobFinished) {
        return d_func()->plugged;
    }
    qWarning() << "isPlugged called without having called start";
    return false;
}

#include "moc_acpluggedjob.cpp"
