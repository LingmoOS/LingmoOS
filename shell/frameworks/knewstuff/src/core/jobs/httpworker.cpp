/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "httpworker.h"

#include "knewstuff_version.h"
#include "knewstuffcore_debug.h"

#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QThread>

class HTTPWorkerNAM
{
public:
    HTTPWorkerNAM()
    {
        QMutexLocker locker(&mutex);
        const QString cacheLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QStringLiteral("/knewstuff");
        cache.setCacheDirectory(cacheLocation);
        QStorageInfo storageInfo(cacheLocation);
        cache.setMaximumCacheSize(qMin(50 * 1024 * 1024, (int)(storageInfo.bytesTotal() / 1000)));
        nam.setCache(&cache);
    }
    QNetworkAccessManager nam;
    QMutex mutex;

    QNetworkReply *get(const QNetworkRequest &request)
    {
        QMutexLocker locker(&mutex);
        return nam.get(request);
    }

    QNetworkDiskCache cache;
};

Q_GLOBAL_STATIC(HTTPWorkerNAM, s_httpWorkerNAM)

using namespace KNSCore;

class KNSCore::HTTPWorkerPrivate
{
public:
    HTTPWorkerPrivate()
        : jobType(HTTPWorker::GetJob)
        , reply(nullptr)
    {
    }
    HTTPWorker::JobType jobType;
    QUrl source;
    QUrl destination;
    QNetworkReply *reply;
    QUrl redirectUrl;

    QFile dataFile;
};

HTTPWorker::HTTPWorker(const QUrl &url, JobType jobType, QObject *parent)
    : QObject(parent)
    , d(new HTTPWorkerPrivate)
{
    d->jobType = jobType;
    d->source = url;
}

HTTPWorker::HTTPWorker(const QUrl &source, const QUrl &destination, KNSCore::HTTPWorker::JobType jobType, QObject *parent)
    : QObject(parent)
    , d(new HTTPWorkerPrivate)
{
    d->jobType = jobType;
    d->source = source;
    d->destination = destination;
}

HTTPWorker::~HTTPWorker() = default;

void HTTPWorker::setUrl(const QUrl &url)
{
    d->source = url;
}

static void addUserAgent(QNetworkRequest &request)
{
    QString agentHeader = QStringLiteral("KNewStuff/%1").arg(QLatin1String(KNEWSTUFF_VERSION_STRING));
    if (QCoreApplication::instance()) {
        agentHeader += QStringLiteral("-%1/%2").arg(QCoreApplication::instance()->applicationName(), QCoreApplication::instance()->applicationVersion());
    }
    request.setHeader(QNetworkRequest::UserAgentHeader, agentHeader);
    // If the remote supports HTTP/2, then we should definitely be using that
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, true);

    // Assume that no cache expiration time will be longer than a week, but otherwise prefer the cache
    // This is mildly hacky, but if we don't do this, we end up with infinite cache expirations in some
    // cases, which of course isn't really acceptable... See ed62ee20 for a situation where that happened.
    QNetworkCacheMetaData cacheMeta{s_httpWorkerNAM->cache.metaData(request.url())};
    if (cacheMeta.isValid()) {
        const QDateTime nextWeek{QDateTime::currentDateTime().addDays(7)};
        if (cacheMeta.expirationDate().isValid() && cacheMeta.expirationDate() < nextWeek) {
            request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        }
    }
}

void HTTPWorker::startRequest()
{
    if (d->reply) {
        // only run one request at a time...
        return;
    }

    QNetworkRequest request(d->source);
    addUserAgent(request);
    d->reply = s_httpWorkerNAM->get(request);
    connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
    connect(d->reply, &QNetworkReply::finished, this, &HTTPWorker::handleFinished);
    if (d->jobType == DownloadJob) {
        d->dataFile.setFileName(d->destination.toLocalFile());
        connect(this, &HTTPWorker::data, this, &HTTPWorker::handleData);
    }
}

void HTTPWorker::handleReadyRead()
{
    QMutexLocker locker(&s_httpWorkerNAM->mutex);
    if (d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).isNull()) {
        do {
            Q_EMIT data(d->reply->read(32768));
        } while (!d->reply->atEnd());
    }
}

void HTTPWorker::handleFinished()
{
    qCDebug(KNEWSTUFFCORE) << Q_FUNC_INFO << d->reply->url();
    if (d->reply->error() != QNetworkReply::NoError) {
        qCWarning(KNEWSTUFFCORE) << d->reply->errorString();
        if (d->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() > 100) {
            // In this case, we're being asked to wait a bit...
            Q_EMIT httpError(d->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), d->reply->rawHeaderPairs());
        }
        Q_EMIT error(d->reply->errorString());
    }

    // Check if the data was obtained from cache or not
    QString fromCache = d->reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool() ? QStringLiteral("(cached)") : QStringLiteral("(NOT cached)");

    // Handle redirections
    const QUrl possibleRedirectUrl = d->reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != d->redirectUrl) {
        d->redirectUrl = d->reply->url().resolved(possibleRedirectUrl);
        if (d->redirectUrl.scheme().startsWith(QLatin1String("http"))) {
            qCDebug(KNEWSTUFFCORE) << d->reply->url().toDisplayString() << "was redirected to" << d->redirectUrl.toDisplayString() << fromCache
                                   << d->reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            d->reply->deleteLater();
            QNetworkRequest request(d->redirectUrl);
            addUserAgent(request);
            d->reply = s_httpWorkerNAM->get(request);
            connect(d->reply, &QNetworkReply::readyRead, this, &HTTPWorker::handleReadyRead);
            connect(d->reply, &QNetworkReply::finished, this, &HTTPWorker::handleFinished);
            return;
        } else {
            qCWarning(KNEWSTUFFCORE) << "Redirection to" << d->redirectUrl.toDisplayString() << "forbidden.";
        }
    } else {
        qCDebug(KNEWSTUFFCORE) << "Data for" << d->reply->url().toDisplayString() << "was fetched" << fromCache;
    }

    if (d->dataFile.isOpen()) {
        d->dataFile.close();
    }

    d->redirectUrl.clear();
    Q_EMIT completed();
}

void HTTPWorker::handleData(const QByteArray &data)
{
    // It turns out that opening a file and then leaving it hanging without writing to it immediately will, at times
    // leave you with a file that suddenly (seemingly magically) no longer exists. Thanks for that.
    if (!d->dataFile.isOpen()) {
        if (d->dataFile.open(QIODevice::WriteOnly)) {
            qCDebug(KNEWSTUFFCORE) << "Opened file" << d->dataFile.fileName() << "for writing.";
        } else {
            qCWarning(KNEWSTUFFCORE) << "Failed to open file for writing!";
            Q_EMIT error(QStringLiteral("Failed to open file %1 for writing!").arg(d->destination.toLocalFile()));
        }
    }
    qCDebug(KNEWSTUFFCORE) << "Writing" << data.length() << "bytes of data to" << d->dataFile.fileName();
    quint64 written = d->dataFile.write(data);
    if (d->dataFile.error()) {
        qCDebug(KNEWSTUFFCORE) << "File has error" << d->dataFile.errorString();
    }
    qCDebug(KNEWSTUFFCORE) << "Wrote" << written << "bytes. File is now size" << d->dataFile.size();
}

#include "moc_httpworker.cpp"
