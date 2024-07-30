/*
 * SPDX-FileCopyrightText: 2018 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediaendpointconnector.h"

#include <QCoreApplication>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QDebug>

#include "adapter.h"
#include "device.h"
#include "initmanagerjob.h"
#include "media.h"
#include "mediaendpoint.h"
#include "mediatransport.h"
#include "services.h"
#include "tpendingcall.h"

using namespace BluezQt;

class NoInputNoOutputAgentPrivate
{
public:
    QStringList allowedUuids;
};

NoInputNoOutputAgent::NoInputNoOutputAgent(const QStringList &uuids, QObject *parent)
    : Agent(parent)
    , d(new NoInputNoOutputAgentPrivate)
{
    d->allowedUuids = uuids;
}

NoInputNoOutputAgent::~NoInputNoOutputAgent() = default;

QDBusObjectPath NoInputNoOutputAgent::objectPath() const
{
    return QDBusObjectPath("/Agent/NoInputNoOutput");
}

Agent::Capability NoInputNoOutputAgent::capability() const
{
    return Agent::NoInputNoOutput;
}

void NoInputNoOutputAgent::authorizeService(DevicePtr device, const QString &uuid, const Request<> &request)
{
    Q_UNUSED(device)

    d->allowedUuids.contains(uuid) ? request.accept() : request.reject();
    Q_EMIT serviceAuthorized(device, uuid, d->allowedUuids.contains(uuid));
}

MediaEndpointConnector::MediaEndpointConnector(Manager *manager, QObject *parent)
    : QObject(parent)
    , m_manager(manager)
{
    connect(manager, &Manager::deviceChanged, [this](DevicePtr device) {
        connect(device.data(), &Device::mediaTransportChanged, this, &MediaEndpointConnector::onTransportChanged);
    });

    NoInputNoOutputAgent *agent = new NoInputNoOutputAgent({Services::AdvancedAudioDistribution, Services::AudioVideoRemoteControl});
    connect(agent, &NoInputNoOutputAgent::serviceAuthorized, this, &MediaEndpointConnector::onServiceAuthorized);
    manager->registerAgent(agent);
    manager->requestDefaultAgent(agent);

    MediaEndpoint *sbcSink = new MediaEndpoint({MediaEndpoint::Role::AudioSink, MediaEndpoint::Codec::Sbc}, manager);
    MediaEndpoint *aacSink = new MediaEndpoint({MediaEndpoint::Role::AudioSink, MediaEndpoint::Codec::Aac}, manager);
    connect(sbcSink, &MediaEndpoint::configurationSelected, this, &MediaEndpointConnector::onConfigurationSelected);
    connect(aacSink, &MediaEndpoint::configurationSelected, this, &MediaEndpointConnector::onConfigurationSelected);
    connect(sbcSink, &MediaEndpoint::configurationSet, this, &MediaEndpointConnector::onConfigurationSet);
    connect(aacSink, &MediaEndpoint::configurationSet, this, &MediaEndpointConnector::onConfigurationSet);
    connect(sbcSink, &MediaEndpoint::configurationCleared, this, &MediaEndpointConnector::onConfigurationCleared);
    connect(aacSink, &MediaEndpoint::configurationCleared, this, &MediaEndpointConnector::onConfigurationCleared);
    manager->usableAdapter()->media()->registerEndpoint(sbcSink);
    manager->usableAdapter()->media()->registerEndpoint(aacSink);
}

void MediaEndpointConnector::onTransportChanged(MediaTransportPtr transport)
{
    if (!transport) {
        return;
    }

    connect(transport.data(), &MediaTransport::stateChanged, [transport](MediaTransport::State state) {
        qDebug() << "Transport state:" << state;

        if (state == MediaTransport::State::Pending) {
            TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t> *fd = transport->tryAcquire();
            connect(fd, &PendingCall::finished, [fd]() {
                qDebug() << "fd: " << fd->valueAt<0>().fileDescriptor() << "mtu read:" << fd->valueAt<1>() << "mtu write:" << fd->valueAt<2>();
            });
        }
    });
    connect(transport.data(), &MediaTransport::volumeChanged, [](quint16 volume) {
        qDebug() << "Transport volume:" << volume;
    });
}

void MediaEndpointConnector::onServiceAuthorized(BluezQt::DevicePtr device, const QString &uuid, bool allowed)
{
    qDebug() << (allowed ? "Accepted" : "Rejected") << "service:" << uuid << "from" << device->friendlyName();
}

void MediaEndpointConnector::onConfigurationSelected(const QByteArray &capabilities, const QByteArray &configuration)
{
    if (configuration.isEmpty()) {
        qDebug() << "No usable configuration found for capabilities:" << capabilities;
    } else {
        qDebug() << "Selected configuration:" << configuration << "for capabilities:" << capabilities;
    }
}

void MediaEndpointConnector::onConfigurationSet(const QString &transportObjectPath, const QVariantMap &properties)
{
    qDebug() << "Set configuration for transport:" << transportObjectPath << "to:" << properties;
}

void MediaEndpointConnector::onConfigurationCleared(const QString &transportObjectPath)
{
    qDebug() << "Cleared configuration for transport:" << transportObjectPath;
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    qDebug() << "Waiting for bluetooth audio source to connect. Ctrl + C to cancel...";

    Manager *manager = new Manager();
    InitManagerJob *initJob = manager->init();
    initJob->exec();
    if (initJob->error()) {
        qWarning() << "Error initializing manager:" << initJob->errorText();
        return 1;
    }

    if (!manager->usableAdapter()) {
        qWarning() << "No usable adapter";
        return 2;
    }

    if (!manager->usableAdapter()->media()) {
        qWarning() << "No media interface";
        return 2;
    }

    new MediaEndpointConnector(manager);

    return app.exec();
}

#include "moc_mediaendpointconnector.cpp"
