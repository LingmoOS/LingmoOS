/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <KIO/StoredTransferJob>
#include <KIO/TransferJob>
#include <KJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QDebug>
#include <QJsonArray>
#include <QStandardPaths>
#include <QUrl>
#include <purpose/pluginbase.h>

// Taken from "share" Data Engine
// key associated with plasma-devel@kde.org
// thanks to Alan Schaaf of Pastebin (alan@pastebin.com)
Q_GLOBAL_STATIC_WITH_ARGS(QByteArray, apiKey, ("0c8b6add8e0f6d53f61fe5ce870a1afa"))

class PastebinJob : public Purpose::Job
{
    Q_OBJECT
public:
    PastebinJob(QObject *parent)
        : Purpose::Job(parent)
        , m_pendingJobs(0)
    {
    }

    void start() override
    {
        const QJsonArray urls = data().value(QLatin1String("urls")).toArray();

        if (urls.isEmpty()) {
            qWarning() << "no urls to share" << urls << data();
            emitResult();
            return;
        }

        m_pendingJobs = 0;
        for (const QJsonValue &val : urls) {
            QString u = val.toString();
            KIO::StoredTransferJob *job = KIO::storedGet(QUrl(u), KIO::NoReload, KIO::HideProgressInfo);
            connect(job, &KJob::finished, this, &PastebinJob::fileFetched);
            m_pendingJobs++;
        }
        Q_ASSERT(m_pendingJobs > 0);
    }

    void fileFetched(KJob *j)
    {
        KIO::StoredTransferJob *job = qobject_cast<KIO::StoredTransferJob *>(j);
        m_data += job->data();
        --m_pendingJobs;

        if (job->error()) {
            setError(job->error());
            setErrorText(job->errorString());
            emitResult();
        } else if (m_pendingJobs == 0) {
            performUpload();
        }
    }

    void performUpload()
    {
        if (m_data.isEmpty()) {
            setError(1);
            setErrorText(i18n("No information to send"));
            emitResult();
            return;
        }

        //             qCDebug(PLUGIN_PASTEBIN) << "exporting patch to pastebin" << source->file();
        QByteArray bytearray =
            "api_option=paste&api_paste_private=1&api_paste_name=kde-purpose-pastebin-plugin&api_paste_expire_date=1D&api_paste_format=diff&api_dev_key="
            + *apiKey + "&api_paste_code=";
        bytearray += QUrl::toPercentEncoding(QString::fromUtf8(m_data));

        const QUrl url(QStringLiteral("https://pastebin.com/api/api_post.php"));

        KIO::TransferJob *tf = KIO::http_post(url, bytearray);

        tf->addMetaData(QStringLiteral("content-type"), QStringLiteral("Content-Type: application/x-www-form-urlencoded"));
        connect(tf, &KIO::TransferJob::data, this, [this](KIO::Job *, const QByteArray &data) {
            m_resultData += data;
        });
        connect(tf, &KJob::result, this, &PastebinJob::textUploaded);

        m_resultData.clear();
    }

    void textUploaded(KJob *job)
    {
        if (job->error()) {
            setError(error());
            setErrorText(job->errorText());
        } else if (!m_resultData.startsWith("http")) {
            setError(1);
            setErrorText(QString::fromUtf8(m_resultData));
        } else {
            setOutput({{QStringLiteral("url"), QString::fromUtf8(m_resultData)}});
        }
        emitResult();
    }

private:
    int m_pendingJobs;
    QByteArray m_data;
    QByteArray m_resultData;
};

class PastebinPlugin : public Purpose::PluginBase
{
    Q_OBJECT
public:
    using PluginBase::PluginBase;
    Purpose::Job *createJob() const override
    {
        return new PastebinJob(nullptr);
    }
};

K_PLUGIN_CLASS_WITH_JSON(PastebinPlugin, "pastebinplugin.json")

#include "pastebinplugin.moc"
