/*
    SPDX-FileCopyrightText: 2016 Dan Leinir Turthra Jensen <admin@leinir.dk>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef HTTPWORKER_H
#define HTTPWORKER_H

#include <QNetworkReply>
#include <QUrl>

class QNetworkReply;
namespace KNSCore
{
class HTTPWorkerPrivate;
class HTTPWorker : public QObject
{
    Q_OBJECT
public:
    enum JobType {
        GetJob,
        DownloadJob, // Much the same as a get... except with a filesystem destination, rather than outputting data
    };
    explicit HTTPWorker(const QUrl &url, JobType jobType = GetJob, QObject *parent = nullptr);
    explicit HTTPWorker(const QUrl &source, const QUrl &destination, JobType jobType = DownloadJob, QObject *parent = nullptr);
    ~HTTPWorker() override;

    void startRequest();

    void setUrl(const QUrl &url);

    Q_SIGNAL void error(QString error);
    Q_SIGNAL void progress(qlonglong current, qlonglong total);
    Q_SIGNAL void completed();
    Q_SIGNAL void data(const QByteArray &data);

    /**
     * Fired in case there is a http error reported
     * In some instances this is useful information for our users, and we want to make sure we report this centrally
     * @param status The HTTP status code (fired in cases where it is perceived by QNetworkReply as an error)
     * @param rawHeaders The raw HTTP headers for the errored-out network request
     */
    Q_SIGNAL void httpError(int status, QList<QNetworkReply::RawHeaderPair> rawHeaders);

    Q_SLOT void handleReadyRead();
    Q_SLOT void handleFinished();
    Q_SLOT void handleData(const QByteArray &data);

private:
    const std::unique_ptr<HTTPWorkerPrivate> d;
};

}

#endif // HTTPWORKER_H
