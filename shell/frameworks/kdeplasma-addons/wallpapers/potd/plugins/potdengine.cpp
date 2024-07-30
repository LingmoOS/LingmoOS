/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "potdengine.h"

#include <chrono>

#include <QDBusConnection>
#include <QFileInfo>
#include <QThreadPool>

#include <KPluginFactory>

#include "cachedprovider.h"
#include "debug.h"

using namespace std::chrono_literals;

namespace
{
constinit bool s_networkInformationAvailable = false;

bool isUsingMeteredConnection()
{
    if (!s_networkInformationAvailable) {
        return false;
    }
    const auto instance = QNetworkInformation::instance();
    if (instance->supports(QNetworkInformation::Feature::Metered)) {
        return instance->isMetered();
    } else if (instance->supports(QNetworkInformation::Feature::TransportMedium)) {
        const auto transport = instance->transportMedium();
        return transport == QNetworkInformation::TransportMedium::Cellular //
            || transport == QNetworkInformation::TransportMedium::Bluetooth;
    }
    return false;
}

bool isNetworkConnected()
{
    if (!s_networkInformationAvailable) {
        return true;
    }
    const auto instance = QNetworkInformation::instance();
    if (instance->supports(QNetworkInformation::Feature::Reachability) && instance->reachability() != QNetworkInformation::Reachability::Online) {
        return false;
    }
    return true;
}
}

PotdClient::PotdClient(const KPluginMetaData &metadata, const QVariantList &args, QObject *parent)
    : QObject(parent)
    , m_metadata(metadata)
    , m_identifier(metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")))
    , m_args(args)
{
    // updateSource() will be called in PotdClient::setUpdateOverMeteredConnection(bool)
    // or PotdBackend::setUpdateOverMeteredConnection(bool)
}

void PotdClient::updateSource(bool refresh)
{
    if (m_loading) {
        return;
    }

    setLoading(true);

    // Check whether it is cached already...
    // Use cache even if it's outdated when using metered connection
    const bool ignoreAge = m_doesUpdateOverMeteredConnection == 0 && isUsingMeteredConnection();
    if ((!refresh || ignoreAge /* Allow force refresh only when no cached image is available */) && CachedProvider::isCached(m_identifier, m_args, ignoreAge)) {
        qCDebug(WALLPAPERPOTD) << "A local cache is available for" << m_identifier << "with arguments" << m_args;

        CachedProvider *provider = new CachedProvider(m_identifier, m_args, this);
        connect(provider, &PotdProvider::finished, this, &PotdClient::slotFinished);
        connect(provider, &PotdProvider::error, this, &PotdClient::slotError);
        return;
    }

    if (auto url = CachedProvider::identifierToPath(m_identifier, m_args); QFileInfo::exists(url)) {
        setLocalUrl(url);
    }

    const auto pluginResult = KPluginFactory::instantiatePlugin<PotdProvider>(m_metadata, this, m_args);

    if (pluginResult) {
        qCDebug(WALLPAPERPOTD) << "Downloading wallpaper from" << m_identifier << m_args;
        connect(pluginResult.plugin, &PotdProvider::finished, this, &PotdClient::slotFinished);
        connect(pluginResult.plugin, &PotdProvider::error, this, &PotdClient::slotError);
    } else {
        qCWarning(WALLPAPERPOTD) << "Error loading PoTD plugin:" << pluginResult.errorString;
    }
}

void PotdClient::setUpdateOverMeteredConnection(int value)
{
    // Don't return if values are the same because there can be multiple
    // backends. Instead, let updateSource() decide whether to update
    // the wallpaper.

    m_doesUpdateOverMeteredConnection = value;
    updateSource();
}

void PotdClient::slotFinished(PotdProvider *provider, const QImage &image)
{
    setInfoUrl(provider->infoUrl());
    setRemoteUrl(provider->remoteUrl());
    setTitle(provider->title());
    setAuthor(provider->author());

    // Store in cache if it's not the response of a CachedProvider
    if (qobject_cast<CachedProvider *>(provider) == nullptr) {
        PotdProviderData data;
        data.remoteUrl = provider->remoteUrl();
        data.infoUrl = provider->infoUrl();
        data.title = provider->title();
        data.author = provider->author();
        data.remoteUrl = provider->remoteUrl();
        data.image = image;

        SaveImageThread *thread = new SaveImageThread(m_identifier, m_args, data);
        connect(thread, &SaveImageThread::done, this, &PotdClient::slotCached);
        QThreadPool::globalInstance()->start(thread);
    } else {
        // Is cache provider
        setLocalUrl(provider->localPath());
        setLoading(false);
        Q_EMIT done(this, true);
    }

    provider->deleteLater();
}

void PotdClient::slotError(PotdProvider *provider)
{
    qCWarning(WALLPAPERPOTD) << m_identifier << "with arguments" << m_args
                             << "failed to fetch the remote wallpaper. Please check your Internet connection or system date.";
    provider->deleteLater();
    setLoading(false);
    Q_EMIT done(this, false);
}

void PotdClient::slotCached(const QString &localPath)
{
    setLocalUrl(localPath);
    setLoading(false);
    Q_EMIT done(this, true);
}

void PotdClient::setLoading(bool status)
{
    if (status == m_loading) {
        return;
    }

    m_loading = status;
    Q_EMIT loadingChanged();
}

void PotdClient::setLocalUrl(const QString &urlString)
{
    if (m_localPath == urlString) {
        return;
    }

    m_localPath = urlString;
    Q_EMIT localUrlChanged();
}

void PotdClient::setInfoUrl(const QUrl &url)
{
    if (m_infoUrl == url) {
        return;
    }

    m_infoUrl = url;
    Q_EMIT infoUrlChanged();
}

void PotdClient::setRemoteUrl(const QUrl &url)
{
    if (m_remoteUrl == url) {
        return;
    }

    m_remoteUrl = url;
    Q_EMIT remoteUrlChanged();
}

void PotdClient::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    Q_EMIT titleChanged();
}

void PotdClient::setAuthor(const QString &author)
{
    if (m_author == author) {
        return;
    }

    m_author = author;
    Q_EMIT authorChanged();
}

PotdEngine::PotdEngine(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<PotdProviderData>();

    loadPluginMetaData();

    connect(&m_checkDatesTimer, &QTimer::timeout, this, &PotdEngine::forceUpdateSource);

    int interval = QDateTime::currentDateTime().msecsTo(QDate::currentDate().addDays(1).startOfDay()) + 60000;
    m_checkDatesTimer.setInterval(interval);
    m_checkDatesTimer.start();
    qCDebug(WALLPAPERPOTD) << "Time to next update (h):" << m_checkDatesTimer.interval() / 1000.0 / 60.0 / 60.0;

    // Sleep checker
    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(slotPrepareForSleep(bool)));
    loadNetworkInformation();
}

PotdClient *PotdEngine::registerClient(const QString &identifier, const QVariantList &args)
{
    auto pr = m_clientMap.equal_range(identifier);

    auto createClient = [this, &identifier, &args]() -> PotdClient * {
        auto pluginIt = m_providersMap.find(identifier);

        if (pluginIt == m_providersMap.end()) {
            // Not a valid identifier
            return nullptr;
        }

        qCDebug(WALLPAPERPOTD) << identifier << "is registered with arguments" << args;
        auto client = new PotdClient(pluginIt->second, args, this);
        m_clientMap.emplace(identifier, ClientPair{client, 1});

        return client;
    };

    while (pr.first != pr.second) {
        // find exact match
        if (pr.first->second.client->m_args == args) {
            pr.first->second.instanceCount++;
            qCDebug(WALLPAPERPOTD) << identifier << "is registered with arguments" << args << "Total client(s):" << pr.first->second.instanceCount;
            return pr.first->second.client;
        }

        pr.first++;
    }

    return createClient();
}

void PotdEngine::unregisterClient(const QString &identifier, const QVariantList &args)
{
    auto [beginIt, endIt] = m_clientMap.equal_range(identifier);

    while (beginIt != endIt) {
        // find exact match
        if (beginIt->second.client->m_args == args) {
            beginIt->second.instanceCount--;
            qCDebug(WALLPAPERPOTD) << identifier << "with arguments" << args << "is unregistered. Remaining client(s):" << beginIt->second.instanceCount;
            if (!beginIt->second.instanceCount) {
                delete beginIt->second.client;
                m_clientMap.erase(beginIt);
                qCDebug(WALLPAPERPOTD) << identifier << "with arguments" << args << "is freed.";
                break;
            }
        }

        beginIt++;
    }
}

void PotdEngine::updateSource(bool refresh)
{
    if (!isNetworkConnected()) {
        qCDebug(WALLPAPERPOTD) << "Network is not connected, so the backend will not update wallpapers.";
        return;
    }

    m_lastUpdateSuccess = true;

    for (const auto &[_, clientPair] : std::as_const(m_clientMap)) {
        if (clientPair.client->m_loading) {
            continue;
        }

        connect(clientPair.client, &PotdClient::done, this, &PotdEngine::slotDone);
        m_updateCount++;
        qCDebug(WALLPAPERPOTD) << clientPair.client->m_metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier")) << "starts updating wallpaper.";
        clientPair.client->updateSource(refresh);
    }
}

void PotdEngine::forceUpdateSource()
{
    updateSource(true);
}

void PotdEngine::slotDone(PotdClient *client, bool success)
{
    disconnect(client, &PotdClient::done, this, &PotdEngine::slotDone);

    qCDebug(WALLPAPERPOTD) << client->m_identifier << "with arguments" << client->m_args << (success ? "finished" : "failed")
                           << "updating the wallpaper. Remaining clients:" << m_updateCount - 1;

    if (!success) {
        m_lastUpdateSuccess = false;
    }

    if (!--m_updateCount) {
        // Do not update until next day, and delay 1minute to make sure last modified condition is satisfied.
        if (m_lastUpdateSuccess) {
            m_checkDatesTimer.setInterval(QDateTime::currentDateTime().msecsTo(QDate::currentDate().startOfDay().addDays(1)) + 60000);
        } else {
            m_checkDatesTimer.setInterval(10min);
        }
        m_checkDatesTimer.start();
        qCDebug(WALLPAPERPOTD) << "Time to next update (h):" << m_checkDatesTimer.interval() / 1000.0 / 60.0 / 60.0;
    }
}

void PotdEngine::slotPrepareForSleep(bool sleep)
{
    if (sleep) {
        return;
    }

    // Resume from sleep
    // Always force update to work around the current date not being updated
    forceUpdateSource();
}

void PotdEngine::slotReachabilityChanged(QNetworkInformation::Reachability newReachability)
{
    if (newReachability == QNetworkInformation::Reachability::Online) {
        qCDebug(WALLPAPERPOTD) << "Network is connected.";
        updateSource(false);
    }
}

void PotdEngine::slotIsMeteredChanged(bool isMetered)
{
    if (isMetered) {
        return;
    }

    updateSource(false);
}

void PotdEngine::loadNetworkInformation()
{
    if (!QNetworkInformation::loadDefaultBackend()) {
        return;
    }

    s_networkInformationAvailable = true;
    const auto instance = QNetworkInformation::instance();

    if (instance->supports(QNetworkInformation::Feature::Metered)) {
        connect(instance, &QNetworkInformation::isMeteredChanged, this, &PotdEngine::slotIsMeteredChanged);
    }
    if (instance->supports(QNetworkInformation::Feature::Reachability)) {
        connect(instance, &QNetworkInformation::reachabilityChanged, this, &PotdEngine::slotReachabilityChanged);
    }
}

void PotdEngine::loadPluginMetaData()
{
    const auto plugins = KPluginMetaData::findPlugins(QStringLiteral("potd"));

    m_providersMap.clear();
    m_providersMap.reserve(plugins.size());

    for (const KPluginMetaData &metadata : plugins) {
        const QString identifier = metadata.value(QStringLiteral("X-KDE-PlasmaPoTDProvider-Identifier"));
        if (!identifier.isEmpty()) {
            m_providersMap.emplace(identifier, metadata);
        }
    }
}
