// SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
// SPDX-FileCopyrightText: 2008 Anne-Marie Mahfouf <annma@kde.org>
// SPDX-FileCopyrightText: 2008 Georges Toth <gtoth@trypill.org>
// SPDX-FileCopyrightText: 2021 Guo Yunhe <i@guoyunhe.me>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "potdprovider.h"

#include <QDate>
#include <QXmlStreamReader>

#include <KJob>

/**
 * This class grabs a random image from the flickr
 * interestingness stream of pictures, for the given date.
 * Should there be no image for the current date, it tries
 * to grab one from the day before yesterday.
 */
class FlickrProvider : public PotdProvider
{
    Q_OBJECT

public:
    explicit FlickrProvider(QObject *parent, const KPluginMetaData &data, const QVariantList &args);

Q_SIGNALS:
    void configLoaded(const QString &apiKey, const QString &apiSecret);

private Q_SLOTS:
    void configRequestFinished(KJob *job);
    void configWriteFinished(KJob *job);

private:
    void loadConfig();
    void refreshConfig();

    void sendXmlRequest(const QString &apiKey);
    void xmlRequestFinished(KJob *job);
    void imageRequestFinished(KJob *job);

    /**
     * Parse the author from the HTML source
     */
    void pageRequestFinished(KJob *job);

private:
    QString m_configLocalPath;
    QUrl m_configRemoteUrl;
    QUrl m_configLocalUrl;
    QDate mActualDate;
    QString mApiKey;
    bool m_refreshed = false;

    QImage m_image;

    QXmlStreamReader xml;

    int mFailureNumber = 0;

    struct PhotoEntry {
        QString urlString;
        QString title;
        QString userId;
        QString photoId;
    };
    std::vector<PhotoEntry> m_photoList;
};
