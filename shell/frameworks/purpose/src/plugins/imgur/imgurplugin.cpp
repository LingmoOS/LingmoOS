/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "mpform.h"
#include <KIO/StoredTransferJob>
#include <KIO/TransferJob>
#include <KJob>
#include <KLocalizedString>
#include <KNotification>
#include <KPluginFactory>
#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <purpose/pluginbase.h>

Q_GLOBAL_STATIC_WITH_ARGS(const QUrl, imageImgurUrl, (QLatin1String("https://api.imgur.com/3/image")))
Q_GLOBAL_STATIC_WITH_ARGS(const QUrl, albumImgurUrl, (QLatin1String("https://api.imgur.com/3/album")))

// key associated with aleixpol@kde.org
Q_GLOBAL_STATIC_WITH_ARGS(const QString, YOUR_CLIENT_ID, (QLatin1String("0bffa5b4ac8383c")))

class ImgurShareJob : public Purpose::Job
{
    Q_OBJECT
public:
    explicit ImgurShareJob(QObject *parent)
        : Purpose::Job(parent)
        , m_pendingJobs(0)
    {
    }

    void start() override
    {
        m_pendingJobs = 0;
        const QJsonArray urls = data().value(QLatin1String("urls")).toArray();
        if (urls.isEmpty()) {
            qWarning() << "no urls to share" << urls << data();
            emitResult();
            return;
        }

        if (urls.count() > 1) {
            KIO::TransferJob *tJob = KIO::storedHttpPost("", *albumImgurUrl, KIO::HideProgressInfo);
            tJob->setMetaData(QMap<QString, QString>{{QStringLiteral("customHTTPHeader"), QStringLiteral("Authorization: Client-ID ") + *YOUR_CLIENT_ID}});
            connect(tJob, &KJob::result, this, &ImgurShareJob::albumCreated);
        } else {
            startUploading();
        }
    }

    QJsonObject processResponse(KJob *job)
    {
        KIO::StoredTransferJob *sjob = qobject_cast<KIO::StoredTransferJob *>(job);
        QJsonParseError error;
        const QJsonObject resultMap = QJsonDocument::fromJson(sjob->data(), &error).object();
        if (sjob->isErrorPage()) {
            setError(3);
            setErrorText(i18n("Error page returned"));
        } else if (job->error()) {
            setError(job->error());
            setErrorText(job->errorText());
        } else if (error.error) {
            setError(1);
            setErrorText(error.errorString());
        } else if (!resultMap.value(QLatin1String("success")).toBool()) {
            setError(2);
            const QJsonObject dataMap = resultMap[QLatin1String("data")].toObject();
            setErrorText(dataMap[QLatin1String("error")].toString());
        } else {
            return resultMap[QLatin1String("data")].toObject();
        }
        emitResult();
        return {};
    }

    void albumCreated(KJob *job)
    {
        const QJsonObject dataMap = processResponse(job);
        if (!dataMap.isEmpty()) {
            m_albumId = dataMap[QLatin1String("id")].toString();
            m_albumDeleteHash = dataMap[QLatin1String("deletehash")].toString();
            startUploading();
        }
    }

    void startUploading()
    {
        Q_EMIT infoMessage(this, i18n("Uploading files to imgur..."));
        const QJsonArray urls = data().value(QLatin1String("urls")).toArray();
        for (const QJsonValue &val : urls) {
            QString u = val.toString();
            KIO::StoredTransferJob *job = KIO::storedGet(QUrl(u));
            connect(job, &KJob::finished, this, &ImgurShareJob::fileFetched);
            m_pendingJobs++;
        }
    }

    void fileFetched(KJob *j)
    {
        if (j->error()) {
            setError(j->error());
            setErrorText(j->errorText());
            emitResult();

            qDebug() << "error:" << j->errorText() << j->errorString();

            return;
        }

        MPForm form;
        KIO::StoredTransferJob *job = qobject_cast<KIO::StoredTransferJob *>(j);
        form.addFile(QStringLiteral("image"), job->url(), job->data());
        form.addPair(QStringLiteral("album"), m_albumDeleteHash, {});
        form.finish();

        KIO::StoredTransferJob *tJob = KIO::storedHttpPost(form.formData(), *imageImgurUrl, KIO::HideProgressInfo);
        tJob->setMetaData(QMap<QString, QString>{
            {QStringLiteral("content-type"), QString::fromLocal8Bit(form.contentType())},
            {QStringLiteral("customHTTPHeader"), QStringLiteral("Authorization: Client-ID ") + *YOUR_CLIENT_ID},
        });
        connect(tJob, &KJob::result, this, &ImgurShareJob::imageUploaded);
    }

    void imageUploaded(KJob *job)
    {
        const QJsonObject dataMap = processResponse(job);
        if (!dataMap.isEmpty()) {
            const QString url = dataMap[QStringLiteral("link")].toString();
            Q_EMIT infoMessage(this, url);
            const QString deletehash = dataMap[QStringLiteral("deletehash")].toString();
            Q_EMIT infoMessage(this, deletehash);
            --m_pendingJobs;

            if (m_pendingJobs == 0) {
                const QString finalUrl = m_albumId.isEmpty() ? url : QStringLiteral("https://imgur.com/a/") + m_albumId;
                const QString deleteUrl = QStringLiteral("https://imgur.com/delete/") + deletehash;

                QGuiApplication::clipboard()->setText(url);
                KNotification::event(KNotification::Notification,
                                     i18n("Imgur Upload"),
                                     i18n("The shared image link (<a href=\"%1\">%1</a>) has been copied to the clipboard.<br><br>If you would like to remove "
                                          "the uploaded image, visit <a href=\"%2\">%2</a>",
                                          finalUrl,
                                          deleteUrl),
                                     KNotification::Persistent);

                emitResult();
            }
        }
    }

private:
    QString m_albumId;
    QString m_albumDeleteHash;
    int m_pendingJobs;
};

class ImgurPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new ImgurShareJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(ImgurPlugin, "imgurplugin.json")

#include "imgurplugin.moc"
