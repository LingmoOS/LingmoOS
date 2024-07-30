/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "youtubejob.h"
#include <KPasswordDialog>

#include <KIO/StoredTransferJob>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

static const QUrl apiUrl(QStringLiteral("https://www.googleapis.com/upload/youtube/v3/videos?part=snippet%2Cstatus&uploadType=resumable"));
static const QString watchUrl = QStringLiteral("https://www.youtube.com/watch?v=");

YoutubeJob::YoutubeJob(const QUrl &url,
                       const QByteArray &accessToken,
                       const QString &title,
                       const QStringList &tags,
                       const QString &description,
                       QObject *parent)
    : KJob(parent)
    , m_url(url)
    , m_token(accessToken)
{
    m_metadata = QByteArray("{ "
        "\"snippet\": {"
            "\"title\": \"" + title.toUtf8() + "\", "
            "\"categoryId\": \"22\", "
            "\"description\": \"" + description.toUtf8() + "\", "
            "\"tags\": [ \"" + tags.join(QStringLiteral("\", \"")).toUtf8() + "\" ] "
        "}, "
        "\"status\": { "
            "\"privacyStatus\": \"public\" "
        "} "
    "}");
}

void YoutubeJob::start()
{
    createLocation();
}

void YoutubeJob::fileFetched(KJob *j)
{
    if (j->error()) {
        setError(j->error());
        setErrorText(j->errorText());
        emitResult();
    }
    KIO::StoredTransferJob *job = qobject_cast<KIO::StoredTransferJob *>(j);

    uploadVideo(job->data());
}

void YoutubeJob::createLocation()
{
    QNetworkRequest req(apiUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json; charset=UTF-8"));
    req.setRawHeader("Authorization", "Bearer " + m_token);
    req.setRawHeader("X-Upload-Content-Type", "video/*");

    auto reply = m_manager.post(req, m_metadata);
    connect(reply, &QNetworkReply::finished, this, &YoutubeJob::locationCreated);
    connect(reply, &QNetworkReply::errorOccurred, this, [](QNetworkReply::NetworkError e) {
        qDebug() << "creation error" << e;
    });
}

void YoutubeJob::locationCreated()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error()) {
        setError(reply->error());
        setErrorText(reply->errorString());
        qWarning() << "couldn't upload file" << reply->readAll();
        emitResult();
        return;
    }
    Q_ASSERT(reply->atEnd());
    Q_ASSERT(reply->hasRawHeader("Location"));

    m_uploadUrl = QUrl::fromEncoded(reply->rawHeader("Location"));

    KIO::StoredTransferJob *job = KIO::storedGet(m_url);
    connect(job, &KJob::finished, this, &YoutubeJob::fileFetched);
}

void YoutubeJob::uploadVideo(const QByteArray &data)
{
    QNetworkRequest req(m_uploadUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("video/*"));
    req.setRawHeader("X-Upload-Content-Length", QByteArray::number(data.size()));
    req.setRawHeader("Authorization", "Bearer " + m_token);

    setTotalAmount(Bytes, data.size());
    auto reply = m_manager.post(req, data);
    connect(reply, &QNetworkReply::finished, this, &YoutubeJob::videoUploaded);
    connect(reply, &QNetworkReply::uploadProgress, this, [this](quint64 bytesSent, quint64 bytesTotal) {
        setProcessedAmount(Bytes, bytesSent);
        setPercent(bytesTotal == 0 ? 0 : (bytesSent * 100) / bytesTotal);
    });
    connect(reply, &QNetworkReply::errorOccurred, this, [](QNetworkReply::NetworkError e) {
        qDebug() << "upload error" << e;
    });
}

void YoutubeJob::videoUploaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply->error()) {
        setError(reply->error());
        setErrorText(reply->errorString());
        qWarning() << "couldn't finish upload" << reply->readAll();
        emitResult();
        return;
    }

    auto doc = QJsonDocument::fromJson(reply->readAll()).object();
    m_output = watchUrl + doc.value(QLatin1String("id")).toString();
    emitResult();
}

#include "moc_youtubejob.cpp"
