/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "filecopyjob.h"

#include "downloadjob.h"
#include "filecopyworker.h"

#include "knewstuffcore_debug.h"

using namespace KNSCore;

class KNSCore::FileCopyJobPrivate
{
public:
    QUrl source;
    QUrl destination;
    int permissions = -1;
    JobFlags flags = DefaultFlags;

    FileCopyWorker *worker = nullptr;
};

FileCopyJob::FileCopyJob(const QUrl &source, const QUrl &destination, int permissions, JobFlags flags, QObject *parent)
    : KJob(parent)
    , d(new FileCopyJobPrivate)
{
    d->source = source;
    d->destination = destination;
    d->permissions = permissions;
    d->flags = flags;
}

FileCopyJob::FileCopyJob(QObject *parent)
    : KJob(parent)
    , d(new FileCopyJobPrivate)
{
}

FileCopyJob::~FileCopyJob() = default;

void FileCopyJob::start()
{
    if (d->worker) {
        // already started...
        return;
    }
    d->worker = new FileCopyWorker(d->source, d->destination, this);
    connect(d->worker, &FileCopyWorker::progress, this, &FileCopyJob::handleProgressUpdate);
    connect(d->worker, &FileCopyWorker::completed, this, &FileCopyJob::handleCompleted);
    connect(d->worker, &FileCopyWorker::error, this, &FileCopyJob::handleError);
    d->worker->start();
}

QUrl FileCopyJob::destUrl() const
{
    return d->destination;
}

QUrl FileCopyJob::srcUrl() const
{
    return d->source;
}

FileCopyJob *FileCopyJob::file_copy(const QUrl &source, const QUrl &destination, int permissions, JobFlags flags, QObject *parent)
{
    FileCopyJob *job = nullptr;
    if (source.isLocalFile() && destination.isLocalFile()) {
        qCDebug(KNEWSTUFFCORE) << "File copy job is local only";
        job = new FileCopyJob(source, destination, permissions, flags, parent);
    } else {
        qCDebug(KNEWSTUFFCORE) << "File copy job is from (or to) a remote URL";
        job = new DownloadJob(source, destination, permissions, flags, parent);
    }
    job->start();
    return job;
}

void FileCopyJob::handleProgressUpdate(qlonglong current, qlonglong total)
{
    setTotalAmount(KJob::Bytes, total);
    setProcessedAmount(KJob::Bytes, current);
    emitPercent(current, total);
}

void FileCopyJob::handleCompleted()
{
    d->worker->deleteLater();
    d->worker = nullptr;
    emitResult();
}

void FileCopyJob::handleError(const QString &errorMessage)
{
    d->worker->deleteLater();
    d->worker = nullptr;
    setError(UserDefinedError);
    setErrorText(errorMessage);
    emitResult();
}

#include "moc_filecopyjob.cpp"
