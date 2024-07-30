/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999-2005 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "readonlypart.h"
#include "readonlypart_p.h"

#include "kparts_logging.h"

#include "guiactivateevent.h"
#include "navigationextension.h"

#include <KIO/FileCopyJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KProtocolInfo>

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QTemporaryFile>

using namespace KParts;

ReadOnlyPart::ReadOnlyPart(QObject *parent, const KPluginMetaData &data)
    : Part(*new ReadOnlyPartPrivate(this, data), parent)
{
}

ReadOnlyPart::ReadOnlyPart(ReadOnlyPartPrivate &dd, QObject *parent)
    : Part(dd, parent)
{
}

ReadOnlyPart::~ReadOnlyPart()
{
    Q_D(ReadOnlyPart);
    d->m_closeUrlFromDestructor = true;
    ReadOnlyPart::closeUrl();
}

QUrl ReadOnlyPart::url() const
{
    Q_D(const ReadOnlyPart);

    return d->m_url;
}

void ReadOnlyPart::setUrl(const QUrl &url)
{
    Q_D(ReadOnlyPart);

    if (d->m_url != url) {
        d->m_url = url;
        if (!d->m_closeUrlFromDestructor) {
            Q_EMIT urlChanged(url);
        }
    }
}

QString ReadOnlyPart::localFilePath() const
{
    Q_D(const ReadOnlyPart);

    return d->m_file;
}

void ReadOnlyPart::setLocalFilePath(const QString &localFilePath)
{
    Q_D(ReadOnlyPart);

    d->m_file = localFilePath;
}

void ReadOnlyPart::setProgressInfoEnabled(bool show)
{
    Q_D(ReadOnlyPart);

    d->m_showProgressInfo = show;
}

bool ReadOnlyPart::isProgressInfoEnabled() const
{
    Q_D(const ReadOnlyPart);

    return d->m_showProgressInfo;
}

bool ReadOnlyPart::openUrl(const QUrl &url)
{
    Q_D(ReadOnlyPart);

    if (!url.isValid()) {
        return false;
    }
    if (d->m_bAutoDetectedMime) {
        d->m_arguments.setMimeType(QString());
        d->m_bAutoDetectedMime = false;
    }
    OpenUrlArguments args = d->m_arguments;
    d->m_closeUrlFromOpenUrl = true;
    const bool closed = closeUrl();
    d->m_closeUrlFromOpenUrl = false;
    if (!closed) {
        return false;
    }
    d->m_arguments = args;
    setUrl(url);

    d->m_file.clear();

    if (d->m_url.isLocalFile()) {
        d->m_file = d->m_url.toLocalFile();
        return d->openLocalFile();
    } else if (KProtocolInfo::protocolClass(url.scheme()) == QLatin1String(":local")) {
        // Maybe we can use a "local path", to avoid a temp copy?
        KIO::JobFlags flags = d->m_showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;
        d->m_statJob = KIO::mostLocalUrl(d->m_url, flags);
        KJobWidgets::setWindow(d->m_statJob, widget());
        connect(d->m_statJob, &KJob::result, this, [d](KJob *job) {
            d->slotStatJobFinished(job);
        });
        return true;
    } else {
        d->openRemoteFile();
        return true;
    }
}

bool ReadOnlyPart::openFile()
{
    qCWarning(KPARTSLOG) << "Default implementation of ReadOnlyPart::openFile called!" << metaObject()->className()
                         << "should reimplement either openUrl or openFile.";
    return false;
}

bool ReadOnlyPartPrivate::openLocalFile()
{
    Q_Q(ReadOnlyPart);
    Q_EMIT q->started(nullptr);
    m_bTemp = false;
    // set the mimetype only if it was not already set (for example, by the host application)
    if (m_arguments.mimeType().isEmpty()) {
        // get the mimetype of the file
        // using findByUrl() to avoid another string -> url conversion
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForUrl(m_url);
        if (!mime.isDefault()) {
            m_arguments.setMimeType(mime.name());
            m_bAutoDetectedMime = true;
        }
    }
    const bool ret = q->openFile();
    if (ret) {
        Q_EMIT q->setWindowCaption(m_url.toDisplayString(QUrl::PreferLocalFile));
        Q_EMIT q->completed();
    } else {
        Q_EMIT q->canceled(QString());
    }
    return ret;
}

void ReadOnlyPartPrivate::openRemoteFile()
{
    Q_Q(ReadOnlyPart);
    m_bTemp = true;
    // Use same extension as remote file. This is important for mimetype-determination (e.g. koffice)
    QString fileName = m_url.fileName();
    QFileInfo fileInfo(fileName);
    QString ext = fileInfo.completeSuffix();
    QString extension;
    if (!ext.isEmpty() && !m_url.hasQuery()) { // not if the URL has a query, e.g. cgi.pl?something
        extension = QLatin1Char('.') + ext; // keep the '.'
    }
    QTemporaryFile tempFile(QDir::tempPath() + QLatin1Char('/') + m_metaData.pluginId() + QLatin1String("XXXXXX") + extension);
    tempFile.setAutoRemove(false);
    tempFile.open();
    m_file = tempFile.fileName();

    QUrl destURL = QUrl::fromLocalFile(m_file);
    KIO::JobFlags flags = m_showProgressInfo ? KIO::DefaultFlags : KIO::HideProgressInfo;
    flags |= KIO::Overwrite;
    m_job = KIO::file_copy(m_url, destURL, 0600, flags);
    m_job->setFinishedNotificationHidden(true);
    KJobWidgets::setWindow(m_job, q->widget());
    Q_EMIT q->started(m_job);

    QObject::connect(m_job, &KJob::result, q, [this](KJob *job) {
        slotJobFinished(job);
    });
    QObject::connect(m_job, &KIO::FileCopyJob::mimeTypeFound, q, [this](KIO::Job *job, const QString &mimeType) {
        slotGotMimeType(job, mimeType);
    });
}

void ReadOnlyPart::abortLoad()
{
    Q_D(ReadOnlyPart);

    if (d->m_statJob) {
        // qDebug() << "Aborting job" << d->m_statJob;
        d->m_statJob->kill();
        d->m_statJob = nullptr;
    }
    if (d->m_job) {
        // qDebug() << "Aborting job" << d->m_job;
        d->m_job->kill();
        d->m_job = nullptr;
    }
}

bool ReadOnlyPart::closeUrl()
{
    Q_D(ReadOnlyPart);

    abortLoad(); // just in case

    d->m_arguments = KParts::OpenUrlArguments();
    if (!d->m_closeUrlFromOpenUrl) {
        setUrl(QUrl());
    }

    if (d->m_bTemp) {
        QFile::remove(d->m_file);
        d->m_bTemp = false;
    }
    // It always succeeds for a read-only part,
    // but the return value exists for reimplementations
    // (e.g. pressing cancel for a modified read-write part)
    return true;
}

void ReadOnlyPartPrivate::slotStatJobFinished(KJob *job)
{
    Q_ASSERT(job == m_statJob);
    m_statJob = nullptr;

    // We could emit canceled on error, but we haven't even emitted started yet,
    // this could maybe confuse some apps? So for now we'll just fallback to KIO::get
    // and error again. Well, maybe this even helps with wrong stat results.
    if (!job->error()) {
        const QUrl localUrl = static_cast<KIO::StatJob *>(job)->mostLocalUrl();
        if (localUrl.isLocalFile()) {
            m_file = localUrl.toLocalFile();
            (void)openLocalFile();
            return;
        }
    }
    openRemoteFile();
}

void ReadOnlyPartPrivate::slotJobFinished(KJob *job)
{
    Q_Q(ReadOnlyPart);

    Q_ASSERT(job == m_job);
    m_job = nullptr;
    if (job->error()) {
        Q_EMIT q->canceled(job->errorString());
    } else {
        if (q->openFile()) {
            Q_EMIT q->setWindowCaption(m_url.toDisplayString(QUrl::PreferLocalFile));
            Q_EMIT q->completed();
        } else {
            Q_EMIT q->canceled(QString());
        }
    }
}

void ReadOnlyPartPrivate::slotGotMimeType(KIO::Job *job, const QString &mime)
{
    // qDebug() << mime;
    Q_ASSERT(job == m_job);
    Q_UNUSED(job)
    // set the mimetype only if it was not already set (for example, by the host application)
    if (m_arguments.mimeType().isEmpty()) {
        m_arguments.setMimeType(mime);
        m_bAutoDetectedMime = true;
    }
}

void ReadOnlyPart::guiActivateEvent(GUIActivateEvent *event)
{
    Q_D(ReadOnlyPart);

    if (event->activated()) {
        if (!d->m_url.isEmpty()) {
            // qDebug() << d->m_url;
            Q_EMIT setWindowCaption(d->m_url.toDisplayString(QUrl::PreferLocalFile));
        } else {
            Q_EMIT setWindowCaption(QString());
        }
    }
}

bool ReadOnlyPart::openStream(const QString &mimeType, const QUrl &url)
{
    Q_D(ReadOnlyPart);

    OpenUrlArguments args = d->m_arguments;
    if (!closeUrl()) {
        return false;
    }
    d->m_arguments = args;
    setUrl(url);
    return doOpenStream(mimeType);
}

bool ReadOnlyPart::writeStream(const QByteArray &data)
{
    return doWriteStream(data);
}

bool ReadOnlyPart::closeStream()
{
    return doCloseStream();
}

NavigationExtension *ReadOnlyPart::navigationExtension() const
{
    return findChild<KParts::NavigationExtension *>();
}

void KParts::ReadOnlyPart::setArguments(const OpenUrlArguments &arguments)
{
    Q_D(ReadOnlyPart);
    d->m_arguments = arguments;
    d->m_bAutoDetectedMime = arguments.mimeType().isEmpty();
}

OpenUrlArguments KParts::ReadOnlyPart::arguments() const
{
    Q_D(const ReadOnlyPart);
    return d->m_arguments;
}

#include "moc_readonlypart.cpp"
