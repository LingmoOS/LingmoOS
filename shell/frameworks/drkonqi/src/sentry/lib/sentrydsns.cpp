// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include "sentrydsns.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>

#include "sentryconnection.h"

using namespace Qt::StringLiterals;

SentryDSNs::SentryDSNs(std::shared_ptr<SentryConnection> connection, Cache cache, QObject *parent)
    : QObject(parent)
    , m_connection(std::move(connection))
    , m_cachePath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/dsns.json"_L1)
    , m_cache(cache == Cache::Yes)
{
    QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
}

void SentryDSNs::load()
{
    if (m_loaded) {
        QMetaObject::invokeMethod(this, &SentryDSNs::loaded, Qt::QueuedConnection);
        return;
    }

    QNetworkRequest request(QUrl("https://autoconfig.kde.org/drkonqi/sentry/0/dsns.json"_L1));
    auto reply = m_connection->get(request);
    connect(reply, &SentryReply::finished, this, [this, reply] {
        reply->deleteLater();

        // We cache the dsns so we have a chance of resolving applications even when offline.
        // Note: not using QNetworkDiskCache since it doesn't add much for a single request
        QFile cacheFile(m_cachePath);
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << reply->error() << reply->errorString();
            if (m_cache) {
                // load cache instead.
                cacheFile.open(QFile::ReadOnly);
            }
            loadData(cacheFile.readAll());
            return;
        }

        const auto payload = reply->readAll();
        if (m_cache) {
            cacheFile.open(QFile::WriteOnly | QFile::Truncate);
            cacheFile.write(payload);
        }
        loadData(payload);
    });
}

SentryDSNContext SentryDSNs::context(const QString &applicationName)
{
    if (m_applicationContexts.contains(applicationName)) {
        return m_applicationContexts.value(applicationName);
    }

    static constexpr auto fallthrough = "fallthrough"_L1;
    if (m_applicationContexts.contains(fallthrough)) {
        return m_applicationContexts.value(fallthrough);
    }

    return {};
}

QUrl SentryDSNContext::dsnUrl() const
{
    return QUrl("https://%1@crash-reports.kde.org/api/%2"_L1.arg(key, index));
}

QUrl SentryDSNContext::envelopeUrl() const
{
    return QUrl("https://%1@crash-reports.kde.org/api/%2/envelope/"_L1.arg(key, index));
}

void SentryDSNs::loadData(const QByteArray &data)
{
    const auto document = QJsonDocument::fromJson(data);
    const auto object = document.object();

    m_applicationContexts.reserve(object.count());
    for (auto it = object.begin(); it != object.end(); it++) {
        const auto &applicationName = it.key();
        const auto &context = it.value().toObject();
        const auto &grabProperty = [context](const QString &key) {
            return context.value(key).toString();
        };
        m_applicationContexts.insert(applicationName,
                                     SentryDSNContext{
                                         .project = grabProperty("project"_L1),
                                         .key = grabProperty("key"_L1),
                                         .index = grabProperty("index"_L1),
                                     });
    }

    m_loaded = true;
    Q_EMIT loaded();
}

#include "moc_sentrydsns.cpp"
