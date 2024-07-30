/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "job.h"
#include "job_p.h"

#include <QEventLoop>

namespace BluezQt
{
JobPrivate::JobPrivate()
{
    eventLoop = nullptr;
    error = Job::NoError;
    running = false;
    finished = false;
    killed = false;
}

Job::Job(QObject *parent)
    : QObject(parent)
    , d_ptr(new JobPrivate)
{
    d_ptr->q_ptr = this;
}

Job::~Job() = default;

void Job::start()
{
    d_func()->running = true;
    QMetaObject::invokeMethod(this, "doStart", Qt::QueuedConnection);
}

void Job::kill()
{
    Q_D(Job);
    Q_ASSERT(!d->eventLoop);

    d->running = false;
    d->finished = true;
    d->killed = true;
    deleteLater();
}

void Job::emitResult()
{
    Q_D(Job);

    if (d->killed) {
        return;
    }

    if (d->eventLoop) {
        d->eventLoop->quit();
    }

    d->running = false;
    d->finished = true;
    doEmitResult();
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

bool Job::isRunning() const
{
    return d_func()->running;
}

bool Job::isFinished() const
{
    return d_func()->finished;
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
    d->running = false;
    d->finished = true;

    return d->error == NoError;
}

} // namespace BluezQt

#include "moc_job.cpp"
