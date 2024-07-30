/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef HTTPJOB_H
#define HTTPJOB_H

#include "jobbase.h"

#include <QNetworkReply>
#include <QUrl>

#include <memory>

namespace KNSCore
{
class HttpJobPrivate;
class HTTPJob : public KJob
{
    Q_OBJECT
public:
    explicit HTTPJob(const QUrl &source, LoadType loadType = Reload, JobFlags flags = DefaultFlags, QObject *parent = nullptr);
    explicit HTTPJob(QObject *parent = nullptr);
    ~HTTPJob() override;

    Q_SLOT void start() override;

    static HTTPJob *get(const QUrl &source, LoadType loadType = Reload, JobFlags flags = DefaultFlags, QObject *parent = nullptr);

Q_SIGNALS:
    /**
     * Data from the worker has arrived.
     * @param job the job that emitted this signal
     * @param data data received from the worker.
     *
     * End of data (EOD) has been reached if data.size() == 0, however, you
     * should not be certain of data.size() == 0 ever happening (e.g. in case
     * of an error), so you should rely on result() instead.
     */
    void data(KJob *job, const QByteArray &data);

    /**
     * Fired in case there is a http error reported
     * In some instances this is useful information for our users, and we want to make sure we report this centrally
     * @param status The HTTP status code (fired in cases where it is perceived by QNetworkReply as an error)
     * @param rawHeaders The raw HTTP headers for the errored-out network request
     */
    void httpError(int status, QList<QNetworkReply::RawHeaderPair> rawHeaders);

protected Q_SLOTS:
    void handleWorkerData(const QByteArray &data);
    void handleWorkerCompleted();
    void handleWorkerError(const QString &error);

private:
    const std::unique_ptr<HttpJobPrivate> d;
};

}

#endif // HTTPJOB_H
