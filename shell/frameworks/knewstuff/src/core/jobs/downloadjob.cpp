/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "downloadjob.h"

#include "httpworker.h"

#include "knewstuffcore_debug.h"

using namespace KNSCore;

class KNSCore::DownloadJobPrivate
{
public:
    DownloadJobPrivate() = default;
    QUrl source;
    QUrl destination;
};

DownloadJob::DownloadJob(const QUrl &source, const QUrl &destination, int permissions, JobFlags flags, QObject *parent)
    : FileCopyJob(source, destination, permissions, flags, parent)
    , d(new DownloadJobPrivate)
{
    d->source = source;
    d->destination = destination;
}

DownloadJob::DownloadJob(QObject *parent)
    : FileCopyJob(parent)
    , d(new DownloadJobPrivate)
{
}

DownloadJob::~DownloadJob() = default;

void DownloadJob::start()
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO;
    HTTPWorker *worker = new HTTPWorker(d->source, d->destination, HTTPWorker::DownloadJob, this);
    connect(worker, &HTTPWorker::completed, this, &DownloadJob::handleWorkerCompleted);
    connect(worker, &HTTPWorker::error, this, &DownloadJob::handleWorkerError);
    worker->startRequest();
}

void DownloadJob::handleWorkerCompleted()
{
    emitResult();
}

void KNSCore::DownloadJob::handleWorkerError(const QString &error)
{
    setError(KJob::UserDefinedError);
    setErrorText(error);
}

#include "moc_downloadjob.cpp"
