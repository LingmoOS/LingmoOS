/*
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef MEDIAENDPOINTCONNECTOR_H
#define MEDIAENDPOINTCONNECTOR_H

#include <QObject>

#include "agent.h"
#include "manager.h"
#include "types.h"

#include <memory>

class NoInputNoOutputAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    /**
     * Creates a new NoInputNoOutputAgent object.
     *
     * @param parent
     */
    explicit NoInputNoOutputAgent(const QStringList &uuids, QObject *parent = nullptr);

    /**
     * Destroys a NoInputNoOutputAgent object.
     */
    ~NoInputNoOutputAgent() override;

Q_SIGNALS:
    /**
     * Indicates that a service was authorized.
     */
    void serviceAuthorized(BluezQt::DevicePtr device, const QString &uuid, bool allowed);

private:
    QDBusObjectPath objectPath() const override;
    Capability capability() const override;
    void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request) override;

    std::unique_ptr<class NoInputNoOutputAgentPrivate> const d;
};

class MediaEndpointConnector : public QObject
{
    Q_OBJECT

public:
    explicit MediaEndpointConnector(BluezQt::Manager *manager, QObject *parent = nullptr);

private:
    void onTransportChanged(BluezQt::MediaTransportPtr transport);
    void onServiceAuthorized(BluezQt::DevicePtr device, const QString &uuid, bool allowed);
    void onConfigurationSelected(const QByteArray &capabilities, const QByteArray &configuration);
    void onConfigurationSet(const QString &transportObjectPath, const QVariantMap &properties);
    void onConfigurationCleared(const QString &transportObjectPath);

    BluezQt::Manager *m_manager;
};

#endif // MEDIAENDPOINTCONNECTOR_H
