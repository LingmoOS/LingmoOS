/*
 *   SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
 *   SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
 *   SPDX-FileCopyrightText: 2016 Weng Xuetian <wengxt@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "wcpotdprovider.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QTextDocumentFragment>
#include <QUrlQuery>

#include <KIO/StoredTransferJob>
#include <KPluginFactory>

WcpotdProvider::WcpotdProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : PotdProvider(parent, data, args)
{
    QUrl url(QStringLiteral("https://commons.wikimedia.org/w/api.php"));

    QUrlQuery urlQuery(url);
    urlQuery.addQueryItem(QStringLiteral("action"), QStringLiteral("parse"));
    urlQuery.addQueryItem(QStringLiteral("text"), QStringLiteral("{{Potd}}"));
    urlQuery.addQueryItem(QStringLiteral("contentmodel"), QStringLiteral("wikitext"));
    urlQuery.addQueryItem(QStringLiteral("format"), QStringLiteral("json"));
    url.setQuery(urlQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    connect(job, &KIO::StoredTransferJob::finished, this, &WcpotdProvider::pageRequestFinished);
}

void WcpotdProvider::pageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }

    const QJsonObject jsonObject = QJsonDocument::fromJson(job->data()).object().value(QLatin1String("parse")).toObject();
    const QJsonArray jsonImageArray = jsonObject.value(QLatin1String("images")).toArray();

    if (jsonImageArray.size() == 0) {
        Q_EMIT error(this);
        return;
    }

    const QString imageFile = jsonImageArray.at(0).toString();
    if (!imageFile.isEmpty()) {
        m_remoteUrl = QUrl(QStringLiteral("https://commons.wikimedia.org/wiki/Special:FilePath/") + imageFile);
        KIO::StoredTransferJob *imageJob = KIO::storedGet(m_remoteUrl, KIO::NoReload, KIO::HideProgressInfo);
        connect(imageJob, &KIO::StoredTransferJob::finished, this, &WcpotdProvider::imageRequestFinished);
    } else {
        Q_EMIT error(this);
        return;
    }

    const QJsonObject jsonTextObject = jsonObject.value(QStringLiteral("text")).toObject();
    const QString text = jsonTextObject.value(QStringLiteral("*")).toString().trimmed();

    if (text.isEmpty()) {
        return;
    }

    /**
     * Match link and title.
     * Example:
     * <div lang="en" dir="ltr" class="description en" style="display:inline;"><a href="https://en.wikipedia.org/wiki/Petros_(Chornohora)" class="extiw"
     * title="w:Petros (Chornohora)">Mount Petros</a>
     */
    const QRegularExpression titleRegEx(QStringLiteral("<div.*?class=\"description.*?>.*?<a href=\"(.+?)\".*?>(.+?)</a>"));
    const QRegularExpressionMatch titleMatch = titleRegEx.match(text);
    if (titleMatch.hasMatch()) {
        m_infoUrl = QUrl(titleMatch.captured(1).trimmed());
        m_title = QTextDocumentFragment::fromHtml(titleMatch.captured(2).trimmed()).toPlainText();
    }
}

void WcpotdProvider::imageRequestFinished(KJob *_job)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(_job);
    if (job->error()) {
        Q_EMIT error(this);
        return;
    }
    QByteArray data = job->data();
    Q_EMIT finished(this, QImage::fromData(data));
}

K_PLUGIN_CLASS_WITH_JSON(WcpotdProvider, "wcpotdprovider.json")

#include "wcpotdprovider.moc"
