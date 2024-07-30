/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "httpjob.h"

#include "httpworker.h"
#include "knewstuffcore_debug.h"

#include <QTimer>

using namespace KNSCore;

class KNSCore::HttpJobPrivate
{
public:
    QUrl source;
    LoadType loadType = Reload;
    JobFlags flags = DefaultFlags;
};

HTTPJob::HTTPJob(const QUrl &source, LoadType loadType, JobFlags flags, QObject *parent)
    : KJob(parent)
    , d(new HttpJobPrivate)
{
    d->source = source;
    d->loadType = loadType;
    d->flags = flags;
}

HTTPJob::HTTPJob(QObject *parent)
    : KJob(parent)
    , d(new HttpJobPrivate)
{
}

HTTPJob::~HTTPJob() = default;

void HTTPJob::start()
{
    HTTPWorker *worker = new HTTPWorker(d->source, HTTPWorker::GetJob, this);
    connect(worker, &HTTPWorker::data, this, &HTTPJob::handleWorkerData);
    connect(worker, &HTTPWorker::completed, this, &HTTPJob::handleWorkerCompleted);
    connect(worker, &HTTPWorker::error, this, &HTTPJob::handleWorkerError);
    connect(worker, &HTTPWorker::httpError, this, &HTTPJob::httpError);
    worker->startRequest();
}

void HTTPJob::handleWorkerData(const QByteArray &data)
{
    Q_EMIT HTTPJob::data(this, data);
}

void HTTPJob::handleWorkerCompleted()
{
    emitResult();
}

void KNSCore::HTTPJob::handleWorkerError(const QString &error)
{
    setError(KJob::UserDefinedError);
    setErrorText(error);
}

HTTPJob *HTTPJob::get(const QUrl &source, LoadType loadType, JobFlags flags, QObject *parent)
{
    HTTPJob *job = new HTTPJob(source, loadType, flags, parent);
    QTimer::singleShot(0, job, &HTTPJob::start);
    return job;
}

#include "moc_httpjob.cpp"
