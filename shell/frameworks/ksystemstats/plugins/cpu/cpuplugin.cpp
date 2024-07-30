/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "cpuplugin.h"
#include "cpuplugin_p.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include <systemstats/SensorContainer.h>

#include "freebsdcpuplugin.h"
#include "linuxcpuplugin.h"

CpuPluginPrivate::CpuPluginPrivate(CpuPlugin *q)
    : m_container(new KSysGuard::SensorContainer(QStringLiteral("cpu"), i18n("CPUs"), q))
{
}

CpuPlugin::CpuPlugin(QObject *parent, const QVariantList &args)
    : SensorPlugin(parent, args)
#if defined Q_OS_LINUX
    , d(new LinuxCpuPluginPrivate(this))
#elif defined Q_OS_FREEBSD
    , d(new FreeBsdCpuPluginPrivate(this))
#else
    , d(new CpuPluginPrivate(this))
#endif
{
}

CpuPlugin::~CpuPlugin() = default;

void CpuPlugin::update()
{
    d->update();
}

K_PLUGIN_CLASS_WITH_JSON(CpuPlugin, "metadata.json")

#include "cpuplugin.moc"

#include "moc_cpuplugin.cpp"
