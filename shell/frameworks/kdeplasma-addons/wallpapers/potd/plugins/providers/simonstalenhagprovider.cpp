/*
 *   SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
 *
 *   SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL
 */

#include "simonstalenhagprovider.h"

#include <random>

#include <QJsonArray>
#include <QJsonDocument>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

static QJsonValue randomArrayValueByKey(const QJsonObject &object, QLatin1String key)
{
    QJsonValue result;

    if (object.isEmpty()) {
        return result;
    }

    auto array = object.value(key).toArray();

    if (array.isEmpty()) {
        return result;
    }

    // Plasma 5.24.0 release date
    std::mt19937 randomEngine(QDate(2022, 2, 3).daysTo(QDate::currentDate()));
    std::uniform_int_distribution<int> distrib(0, array.size() - 1);

    return array.at(distrib(randomEngine));
}

SimonStalenhagProvider::SimonStalenhagProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    const QUrl url(QStringLiteral("https://raw.githubusercontent.com/a-andreyev/simonstalenhag-se-metadata/main/entrypoint.json"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &SimonStalenhagProvider::entrypointRequestFinished);
}

void SimonStalenhagProvider::entrypointRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    auto metadataString = randomArrayValueByKey(json.object(), QLatin1String("simonstalenhag-se-entrypoint"));
    auto urlStr = metadataString.toString();
    if (urlStr.isEmpty()) {
        Q_EMIT error(this);
        return;
    }
    QUrl metaDataUrl(urlStr);
    KIO::StoredTransferJob *metaDataJob = KIO::storedGet(metaDataUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(metaDataJob, &KIO::StoredTransferJob::finished, this, &SimonStalenhagProvider::metaDataRequestFinished);
}

void SimonStalenhagProvider::metaDataRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    auto json = QJsonDocument::fromJson(job->data());
    auto imageObj = randomArrayValueByKey(json.object(), QLatin1String("simonstalenhag.se"));
    auto urlStr = imageObj.toObject().value(QLatin1String("imagebig")).toString();
    if (urlStr.isEmpty()) {
        Q_EMIT error(this);
        return;
    }
    m_remoteUrl = QUrl(urlStr);

    const QString titleStr = imageObj.toObject().value(QStringLiteral("name")).toString();
    const QString sectionStr = imageObj.toObject().value(QStringLiteral("section")).toString();
    if (!titleStr.isEmpty()) {
        if (!sectionStr.isEmpty()) {
            m_title = sectionStr + " - " + titleStr;
        } else {
            m_title = titleStr;
        }
    }

    KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
    connect(imageJob, &KIO::StoredTransferJob::finished, this, &SimonStalenhagProvider::imageRequestFinished);
}

void SimonStalenhagProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }
    QByteArray data = job->data();
    Q_EMIT finished(this, QImage::fromData(data));
}

K_PLUGIN_CLASS_WITH_JSON(SimonStalenhagProvider, "simonstalenhagprovider.json")

#include "simonstalenhagprovider.moc"
