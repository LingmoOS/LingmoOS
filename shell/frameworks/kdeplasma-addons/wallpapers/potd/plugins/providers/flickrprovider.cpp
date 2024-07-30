// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
// SPDX-FileCopyrightText: 2008 Georges Toth <gtoth@trypill.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "flickrprovider.h"

#include <random>

#include <QFileInfo>
#include <QRegularExpression>
#include <QTextDocumentFragment>
#include <QUrlQuery>

#include <KConfigGroup>
#include <KIO/StoredTransferJob>
#include <KPluginFactory>
#include <KSharedConfig>

#include "debug.h"

using namespace Qt::StringLiterals;

static QUrl buildUrl(const QDate &date, const QString &apiKey)
{
    QUrl url(QLatin1String("https://api.flickr.com/services/rest/"));
    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("api_key"), apiKey);
    urlQuery.addQueryItem(QStringLiteral("method"), QStringLiteral("flickr.interestingness.getList"));
    urlQuery.addQueryItem(QStringLiteral("date"), date.toString(Qt::ISODate));
    // url_o might be either too small or too large.
    urlQuery.addQueryItem(QStringLiteral("extras"), QStringLiteral("url_k,url_h,url_o"));
    url.setQuery(urlQuery);

    return url;
}

FlickrProvider::FlickrProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    connect(this, &FlickrProvider::configLoaded, this, &FlickrProvider::sendXmlRequest);

    loadConfig();
}

void FlickrProvider::configRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        qCWarning(WALLPAPERPOTD) << "configRequestFinished error: failed to fetch data";
        Q_EMIT error(this);
        return;
    }

    KIO::StoredTransferJob *putJob = KIO::storedPut(job->data(), m_configLocalUrl, -1);
    connect(putJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::configWriteFinished);
}

void FlickrProvider::configWriteFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        qCWarning(WALLPAPERPOTD) << "configWriteFinished error: failed to write data." << job->errorText();
        Q_EMIT error(this);
    } else {
        loadConfig();
    }
}

void FlickrProvider::loadConfig()
{
    // TODO move to flickr provider
    const QString configFileName = QStringLiteral("%1provider.conf").arg(identifier());
    m_configRemoteUrl = QUrl(QStringLiteral("https://autoconfig.kde.org/potd/") + configFileName);
    m_configLocalPath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/plasma_engine_potd/") + configFileName;
    m_configLocalUrl = QUrl::fromLocalFile(m_configLocalPath);

    auto config = KSharedConfig::openConfig(m_configLocalPath, KConfig::NoGlobals);
    KConfigGroup apiGroup = config->group("API");
    QString apiKey = apiGroup.readEntry("API_KEY");
    QString apiSecret = apiGroup.readEntry("API_SECRET");

    Q_EMIT configLoaded(apiKey, apiSecret);
}

void FlickrProvider::refreshConfig()
{
    // You can only refresh it once in a provider's life cycle
    if (m_refreshed) {
        return;
    }
    // You can only refresh it once in a day
    QFileInfo configFileInfo = QFileInfo(m_configLocalPath);
    if (configFileInfo.exists() && configFileInfo.lastModified().addDays(1) > QDateTime::currentDateTime()) {
        return;
    }

    KIO::StoredTransferJob *job = KIO::storedGet(m_configRemoteUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &FlickrProvider::configRequestFinished);

    m_refreshed = true;
}

void FlickrProvider::sendXmlRequest(const QString &apiKey)
{
    if (apiKey.isNull()) {
        refreshConfig();
        return;
    }

    mApiKey = apiKey;
    mActualDate = QDate::currentDate().addDays(-2);

    const QUrl xmlUrl = buildUrl(mActualDate, apiKey);

    KIO::StoredTransferJob *xmlJob = KIO::storedGet(xmlUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(xmlJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::xmlRequestFinished);
}

void FlickrProvider::xmlRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        qCWarning(WALLPAPERPOTD) << "XML request error:" << job->errorText();
        Q_EMIT error(this);
        return;
    }

    // Clear the list
    m_photoList.clear();
    m_photoList.reserve(100);

    xml.clear();
    xml.addData(job->data());

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            const auto attributes = xml.attributes();
            if (xml.name() == QLatin1String("rsp")) {
                /* no pictures available for the specified parameters */
                if (attributes.value(QLatin1String("stat")) != QLatin1String("ok")) {
                    qCWarning(WALLPAPERPOTD) << "xmlRequestFinished error: no photos for the query";
                    Q_EMIT error(this);
                    return;
                }
            } else if (xml.name() == QLatin1String("photo")) {
                if (attributes.value(QLatin1String("ispublic")) != QLatin1String("1")) {
                    continue;
                }

                constexpr QLatin1String fallbackList[] = {"url_k"_L1, "url_h"_L1};

                bool found = false;
                for (auto urlAttrString : fallbackList) {
                    // Get the best url.
                    if (attributes.hasAttribute(urlAttrString)) {
                        QString title, userId, photoId;
                        if (attributes.hasAttribute(QLatin1String("title"))) {
                            title = QTextDocumentFragment::fromHtml(attributes.value(QLatin1String("title")).toString().trimmed()).toPlainText();
                        }
                        if (attributes.hasAttribute(QLatin1String("owner")) && attributes.hasAttribute(QLatin1String("id"))) {
                            userId = attributes.value(QLatin1String("owner")).toString();
                            photoId = attributes.value(QLatin1String("id")).toString();
                        }
                        m_photoList.emplace_back(PhotoEntry{
                            attributes.value(urlAttrString).toString(),
                            title,
                            userId,
                            photoId,
                        });
                        found = true;
                        break;
                    }
                }

                // The logic here is, if url_h or url_k are present, url_o must
                // has higher quality, otherwise, url_o is worse than k/h size.
                // If url_o is better, prefer url_o.
                if (found) {
                    constexpr QLatin1String originAttr("url_o");
                    if (attributes.hasAttribute(originAttr)) {
                        m_photoList.back().urlString = attributes.value(originAttr).toString();
                    }
                }
            }
        }
    }

    if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
        qCWarning(WALLPAPERPOTD) << "XML ERROR at line" << xml.lineNumber() << xml.error();
    }

    if (m_photoList.begin() != m_photoList.end()) {
        // Plasma 5.24.0 release date
        std::mt19937 randomEngine(QDate(2022, 2, 3).daysTo(QDate::currentDate()));
        std::uniform_int_distribution<int> distrib(0, m_photoList.size() - 1);

        const PhotoEntry &randomPhotoEntry = m_photoList.at(distrib(randomEngine));
        m_remoteUrl = QUrl(randomPhotoEntry.urlString);
        m_title = randomPhotoEntry.title;

        /**
         * Visit the photo page to get the author
         * API document: https://www.flickr.com/services/api/misc.urls.html
         * https://www.flickr.com/photos/{user-id}/{photo-id}
         */
        if (!(randomPhotoEntry.userId.isEmpty() || randomPhotoEntry.photoId.isEmpty())) {
            m_infoUrl = QUrl(QStringLiteral("https://www.flickr.com/photos/%1/%2").arg(randomPhotoEntry.userId, randomPhotoEntry.photoId));
        }

        KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::imageRequestFinished);
    } else {
        qCWarning(WALLPAPERPOTD) << "List is empty in XML file";
        Q_EMIT error(this);
    }
}

void FlickrProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        qCWarning(WALLPAPERPOTD) << "Image request error:" << job->errorText();
        Q_EMIT error(this);
        return;
    }

    m_image = QImage::fromData(job->data());

    // Visit the photo page to get the author
    if (!m_infoUrl.isEmpty()) {
        KIO::StoredTransferJob *pageJob = KIO::storedGet(m_infoUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(pageJob, &KIO::StoredTransferJob::finished, this, &FlickrProvider::pageRequestFinished);
    } else {
        // No information is fine
        Q_EMIT finished(this, m_image);
    }
}

void FlickrProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        qCWarning(WALLPAPERPOTD) << "No author available";
        Q_EMIT finished(this, m_image);
        return;
    }

    const QString data = QString::fromUtf8(job->data()).simplified();

    // Example: <a href="/photos/jellybeanzgallery/" class="owner-name truncate" title="Go to Hammerchewer&#x27;s photostream"
    // data-track="attributionNameClick">Hammerchewer</a>
    QRegularExpression authorRegEx(QStringLiteral("<a.*?class=\"owner-name truncate\".*?>(.+?)</a>"));
    QRegularExpressionMatch match = authorRegEx.match(data);

    if (match.hasMatch()) {
        m_author = QTextDocumentFragment::fromHtml(match.captured(1).trimmed()).toPlainText();
    }

    Q_EMIT finished(this, m_image);
}

K_PLUGIN_CLASS_WITH_JSON(FlickrProvider, "flickrprovider.json")

#include "flickrprovider.moc"
