/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "job.h"
#include "job_p.h"

#include <QEventLoop>

using namespace Solid;
JobPrivate::JobPrivate()
{
    eventLoop = nullptr;
    error = Job::NoError;
}

Job::Job(QObject *parent)
    : QObject(parent)
    , d_ptr(new JobPrivate)
{
    d_ptr->q_ptr = this;
}

Job::~Job()
{
    delete d_ptr;
}

void Job::start()
{
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void Job::emitResult()
{
    Q_D(Job);
    if (d->eventLoop) {
        d->eventLoop->quit();
    }

    Q_EMIT result(this);
    deleteLater();
}

int Job::error() const
{
    return d_func()->error;
}

QString Job::errorText() const
{
    return d_func()->errorText;
}

void Job::setError(int errorCode)
{
    d_func()->error = errorCode;
}

void Job::setErrorText(const QString &errorText)
{
    d_func()->errorText = errorText;
}

bool Job::exec()
{
    Q_D(Job);

    Q_ASSERT(!d->eventLoop);

    QEventLoop loop(this);
    d->eventLoop = &loop;

    start();
    d->eventLoop->exec(QEventLoop::ExcludeUserInputEvents);

    return (d->error == NoError);
}

Job::Job(JobPrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
}

#include "moc_job.cpp"
