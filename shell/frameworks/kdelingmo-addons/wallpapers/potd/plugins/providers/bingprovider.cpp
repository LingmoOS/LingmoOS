/*
 *   SPDX-FileCopyrightText: 2017 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "bingprovider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression> // Extract from the copyright text

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

BingProvider::BingProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
    , m_screenWidth(args.size() >= 2 ? args[0].toInt() : 0)
    , m_screenHeight(args.size() >= 2 ? args[1].toInt() : 0)
{
    const QUrl url(QStringLiteral("https://www.bing.com/HPImageArchive.aspx?format=js&idx=0&n=1"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &BingProvider::pageRequestFinished);
}

void BingProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    do {
        if (json.isNull()) {
            break;
        }
        auto imagesArray = json.object().value(QLatin1String("images"));
        if (!imagesArray.isArray() || imagesArray.toArray().size() <= 0) {
            break;
        }
        auto imageObj = imagesArray.toArray().at(0);
        if (!imageObj.isObject()) {
            break;
        }
        const QJsonObject imageObject = imageObj.toObject();
        auto url = imageObject.value(QLatin1String("urlbase"));
        QString urlString = url.isString() ? url.toString() : QString();
        if (urlString.isEmpty()) {
            break;
        }

        urlString = QStringLiteral("https://www.bing.com/") + urlString;

        if (m_screenWidth > 1920 || m_screenHeight > 1080) {
            // Use 4k wallpaper
            urlString += QStringLiteral("_UHD.jpg");
        } else {
            urlString += QStringLiteral("_1920x1080.jpg");
        }
        m_remoteUrl = QUrl(urlString);

        // Parse the title and the copyright text from the json data
        // Example copyright text: "草丛中的母狮和它的幼崽，南非 (© Andrew Coleman/Getty Images)"
        const QString copyright = imageObject.value(QStringLiteral("copyright")).toString();
        const QRegularExpression copyrightRegEx(QStringLiteral("(.+?)[\\(（](.+?)[\\)）]"));
        if (const QRegularExpressionMatch match = copyrightRegEx.match(copyright); match.hasMatch()) {
            // In some regions "title" is empty, so extract the title from the copyright text.
            m_title = match.captured(1).trimmed();
            m_author = match.captured(2).remove(QStringLiteral("©")).trimmed();
        }

        const QString title = imageObject.value(QStringLiteral("title")).toString();
        if (!title.isEmpty()) {
            m_title = title;
        }

        const QString infoUrl = imageObject.value(QStringLiteral("copyrightlink")).toString();
        if (!infoUrl.isEmpty()) {
            m_infoUrl = QUrl(infoUrl);
        }

        KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &BingProvider::imageRequestFinished);
        return;
    } while (0);

    Q_EMIT error(this);
    return;
}

void BingProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }
    QByteArray data = job->data();
    Q_EMIT finished(this, QImage::fromData(data));
}

K_PLUGIN_CLASS_WITH_JSON(BingProvider, "bingprovider.json")

#include "bingprovider.moc"
