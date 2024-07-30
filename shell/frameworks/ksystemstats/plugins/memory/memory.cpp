/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "memory.h"

#include "backend.h"
#if defined Q_OS_LINUX
#include "linuxbackend.h"
#elif defined Q_OS_FREEBSD
#include "freebsdbackend.h"
#endif

#include <systemstats/SensorContainer.h>

#include <KLocalizedString>
#include <KPluginFactory>

MemoryPlugin::MemoryPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
{
    auto container = new KSysGuard::SensorContainer(QStringLiteral("memory"), i18nc("@title", "Memory"), this);
#if defined Q_OS_LINUX
    m_backend = std::make_unique<LinuxMemoryBackend>(container);
#elif defined Q_OS_FREEBSD
    m_backend = std::make_unique<FreeBsdMemoryBackend>(container);
#endif
    m_backend->initSensors();
}

MemoryPlugin::~MemoryPlugin() = default;

void MemoryPlugin::update()
{
    m_backend->update();
}

K_PLUGIN_CLASS_WITH_JSON(MemoryPlugin, "metadata.json")
#include "memory.moc"

#include "moc_memory.cpp"
