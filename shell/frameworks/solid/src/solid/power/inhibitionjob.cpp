/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "inhibitionjob.h"
#include "backends/abstractinhibitionjob.h"
#include "inhibitionjob_p.h"
#include "powerbackendloader.h"

#include "inhibition.h"

#include <QDebug>
#include <qglobal.h>

using namespace Solid;

InhibitionJobPrivate::InhibitionJobPrivate()
    : inhibitions(Power::None)
    , inhibition(nullptr)
    , backendJob(nullptr)
{
}

InhibitionJob::InhibitionJob(QObject *parent)
    : Job(*new InhibitionJobPrivate(), parent)
{
}

Inhibition *InhibitionJob::inhibition() const
{
    Q_ASSERT_X(d_func()->inhibition, "addInhibitionJob", "::inhibition() called before result() has been emitted");

    if (d_func()->inhibition) {
        return d_func()->inhibition;
    }

    qWarning() << "result() has not been emitted yet, job not finished";
    return nullptr;
}

void InhibitionJob::doStart()
{
    Q_D(InhibitionJob);

    if (!d->inhibitions) {
        setError(InvalidInhibitions);
        emitResult();
        return;
    }
    if (d->description.isEmpty()) {
        setError(EmptyDescription);
        emitResult();
        return;
    }

    d->backendJob = PowerBackendLoader::addInhibitionJob(d->inhibitions, d->description);
    connect(d->backendJob, &AbstractInhibitionJob::result, [this, d]() {
        d_func()->inhibition = d->backendJob->inhibition();
        emitResult();
    });

    d->backendJob->start();
}

void InhibitionJob::setInhibitions(Power::InhibitionTypes inhibitions)
{
    Q_D(InhibitionJob);
    d->inhibitions = inhibitions;
}

Power::InhibitionTypes InhibitionJob::inhibitions() const
{
    return d_func()->inhibitions;
}

void InhibitionJob::setDescription(const QString &description)
{
    Q_D(InhibitionJob);
    d->description = description;
}

QString InhibitionJob::description() const
{
    return d_func()->description;
}

#include "moc_inhibitionjob.cpp"
