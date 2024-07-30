/*
    SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "NetworkPlugin.h"

#include <QDebug>

#include <KLocalizedString>
#include <KPluginFactory>

#include <systemstats/SensorContainer.h>

#include "NetworkDevice.h"
#include "NetworkBackend.h"
#include "AllDevicesObject.h"

#ifdef NETWORKMANAGER_FOUND
#include "NetworkManagerBackend.h"
#endif
#ifdef Q_OS_LINUX
#include "RtNetlinkBackend.h"
#endif

class NetworkPrivate
{
public:
    KSysGuard::SensorContainer *container = nullptr;

    AllDevicesObject *allDevices = nullptr;

    NetworkBackend *backend = nullptr;

    QList<NetworkDevice *> devices;
};

NetworkPlugin::NetworkPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
    , d(std::make_unique<NetworkPrivate>())
{
    d->container = new KSysGuard::SensorContainer(QStringLiteral("network"), i18nc("@title", "Network Devices"), this);

    d->allDevices = new AllDevicesObject(d->container);

    using creationFunction = std::add_pointer_t<NetworkBackend *(NetworkPlugin *parent)>;
    std::vector<creationFunction> backendFunctions;
#ifdef NETWORKMANAGER_FOUND
    backendFunctions.emplace_back([](NetworkPlugin *parent) -> NetworkBackend* {return new NetworkManagerBackend(parent);});
#endif
#ifdef Q_OS_LINUX
    backendFunctions.emplace_back([](NetworkPlugin *parent) -> NetworkBackend* {return new RtNetlinkBackend(parent);});
#endif
    for (auto func : backendFunctions) {
        auto backend = func(this);
        if (backend->isSupported()) {
            d->backend = backend;
            break;
        }
        delete backend;
    }
    if (!d->backend) {
        qWarning() << "Unable to start backend, network information not available.";
        return;
    }

    connect(d->backend, &NetworkBackend::deviceAdded, this, &NetworkPlugin::onDeviceAdded);
    connect(d->backend, &NetworkBackend::deviceRemoved, this, &NetworkPlugin::onDeviceRemoved);

    d->backend->start();
}

void NetworkPlugin::onDeviceAdded(NetworkDevice *device)
{
    d->container->addObject(device);
}

void NetworkPlugin::onDeviceRemoved(NetworkDevice *device)
{
    d->container->removeObject(device);
}

void NetworkPlugin::update()
{
    if (d->backend) {
        d->backend->update();
    }
}

NetworkPlugin::~NetworkPlugin() = default;

K_PLUGIN_CLASS_WITH_JSON(NetworkPlugin, "metadata.json")

#include "NetworkPlugin.moc"

#include "moc_NetworkPlugin.cpp"
