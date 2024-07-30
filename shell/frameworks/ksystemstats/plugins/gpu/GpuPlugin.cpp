/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "GpuPlugin.h"

#include <KPluginFactory>
#include <KLocalizedString>

#include <systemstats/SensorContainer.h>

#include "GpuDevice.h"
#include "LinuxBackend.h"
#include "AllGpus.h"

class GpuPlugin::Private
{
public:
    std::unique_ptr<KSysGuard::SensorContainer> container;
    std::unique_ptr<GpuBackend> backend;

    AllGpus *allGpus = nullptr;
};

GpuPlugin::GpuPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
    , d(std::make_unique<Private>())
{
    d->container = std::make_unique<KSysGuard::SensorContainer>(QStringLiteral("gpu"), i18nc("@title", "GPU"), this);

#ifdef Q_OS_LINUX
    d->backend = std::make_unique<LinuxBackend>();
#endif

    if (d->backend) {
        connect(d->backend.get(), &GpuBackend::deviceAdded, this, [this](GpuDevice* device) {
            d->container->addObject(device);
        });
        connect(d->backend.get(), &GpuBackend::deviceRemoved, this, [this](GpuDevice* device) {
            d->container->removeObject(device);
        });
        d->backend->start();

        if (d->backend->deviceCount() > 0) {
            d->allGpus = new AllGpus(d->container.get());
        }
    }
}

GpuPlugin::~GpuPlugin()
{
    d->container.reset();
    if (d->backend) {
        d->backend->stop();
    }
}

void GpuPlugin::update()
{
    if (d->backend) {
        d->backend->update();
    }
}

K_PLUGIN_CLASS_WITH_JSON(GpuPlugin, "metadata.json")

#include "GpuPlugin.moc"

#include "moc_GpuPlugin.cpp"
