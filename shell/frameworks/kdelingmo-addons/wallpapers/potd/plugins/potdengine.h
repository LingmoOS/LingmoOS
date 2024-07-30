/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <unordered_map>

#include <QDate>
#include <QNetworkInformation>
#include <QObject>
#include <QTimer>

#include <KPluginMetaData>

#include "potdprovider.h"

/**
 * This class provides data for the specific identifier and arguments
 */
class PotdClient : public QObject
{
    Q_OBJECT

public:
    PotdClient(const KPluginMetaData &metadata, const QVariantList &args, QObject *parent = nullptr);

    void updateSource(bool refresh = false);
    void setUpdateOverMeteredConnection(int value);

    KPluginMetaData m_metadata;
    bool m_loading = false;

    QUrl m_remoteUrl;
    QUrl m_infoUrl;
    QString m_localPath;
    QString m_title;
    QString m_author;

Q_SIGNALS:
    /**
     * Emitted only when the image content has been updated
     */
    void loadingChanged();
    void localUrlChanged();
    void infoUrlChanged();
    void remoteUrlChanged();
    void titleChanged();
    void authorChanged();
    void done(PotdClient *client, bool success);

private Q_SLOTS:
    void slotFinished(PotdProvider *provider, const QImage &image);
    void slotError(PotdProvider *provider);
    void slotCached(const QString &localPath);

private:
    void setLoading(bool status);
    void setLocalUrl(const QString &urlString);
    void setInfoUrl(const QUrl &url);
    void setRemoteUrl(const QUrl &url);
    void setTitle(const QString &title);
    void setAuthor(const QString &author);

    QString m_identifier;
    QVariantList m_args;
    int m_doesUpdateOverMeteredConnection = 0;

    friend class PotdEngine;
};

/**
 * This class manages the clients for the Pictures of The Day plugin
 */
class PotdEngine : public QObject
{
    Q_OBJECT

public:
    explicit PotdEngine(QObject *parent = nullptr);

    /**
     * Registers the @p identifier in the engine
     *
     * A client will be created, and will be automatically destroyed when no backend has
     * the identifier.
     *
     * @return the client that relays signals for the specific identifier
     */
    PotdClient *registerClient(const QString &identifier, const QVariantList &args);
    void unregisterClient(const QString &identifier, const QVariantList &args);

    void updateSource(bool refresh);

private Q_SLOTS:
    void forceUpdateSource();
    void slotDone(PotdClient *client, bool success);
    void slotPrepareForSleep(bool sleep);
    void slotReachabilityChanged(QNetworkInformation::Reachability newReachability);
    void slotIsMeteredChanged(bool isMetered);

private:
    void loadNetworkInformation();
    void loadPluginMetaData();

    struct ClientPair {
        PotdClient *const client = nullptr;
        int instanceCount = 0;
    };
    std::unordered_multimap<QString /* identifier */, ClientPair> m_clientMap;
    std::unordered_map<QString, KPluginMetaData> m_providersMap;

    QTimer m_checkDatesTimer;
    int m_updateCount = 0;

    bool m_lastUpdateSuccess = false;
};
